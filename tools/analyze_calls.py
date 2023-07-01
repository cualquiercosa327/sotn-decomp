#Very closely related to analyze_calls.py (and uses some of the same internal Python functions).
#USAGE: Invoke with no arguments to start interactive graphical mode. User will be prompted for
#functions one at a time, and a graph will be made for each one they give.
#Invoke with one argument, a function name, for command line mode. Callers and callees will be printed.

#Credit to xeeynamo for creation of analyze_calls, and sonicdcer for suggestion of command line mode.

#On first run, this will generate a call tree, traversing every function and identifying its calls.
#This gets saved to sotn_calltree.txt. To regenerate this tree, simply delete the file.

#all just for drawing the graph
import graphviz
import matplotlib.pyplot as plt
from PIL import Image
import io
#regex matching, file access
import re
import os
from pathlib import Path
import sys

callable_registers = ["$v0", "$v1", "$a0","$a1","$t2"]

def handle_jal_call(full_file,call_index, known_func_list):
    debug = False#("800F2B08" in full_file[call_index])
    call_line = full_file[call_index]
    call_target = call_line.split(" ")[-1].strip()
    if call_target in known_func_list: #easy, just a direct function call by name
        return call_target
    #Check if we're calling based on a register's value
    if call_target in callable_registers:
        #Find what line sets that register.
        searchback = 1
        while(call_target not in full_file[call_index-searchback] or 'beqz' in full_file[call_index-searchback] or 'jalr' in full_file[call_index-searchback]):
            searchback+=1
        callreg_setline = full_file[call_index-searchback]
        if(debug):
            print(callreg_setline)
        if "g_api" in callreg_setline:
            #regex that will pull out the function being called
            match = re.search(r'%lo\(([^)]+)\)', callreg_setline)
            if match.group(1) in known_func_list:
                return match.group(1)
            if match.group(1) == 'g_api':
                return 'g_api_o_Update'
        if f'lw         {call_target}, %lo(D_' in callreg_setline: #Simply jumping to what's stored in a D_ variable
            jump_variable = callreg_setline[51:61]
            return jump_variable
        #happens in NZ0/func_801C1034. v0 is set by dereferencing a register.
        target_setter_pattern = r'lw\s+' + '\\' + call_target + r', 0x.{,2}\((\$\w+)'
        if match := re.search(target_setter_pattern,callreg_setline):
            source_register = match.group(1)
            pattern = r'\s{3,}'+ '\\' + source_register
            #Now find where THAT was set. Repeat the same we did for v0.
            searchback+=1
            while not re.search(pattern, full_file[call_index-searchback]):
                searchback+=1
            if "g_api" in full_file[call_index-searchback]:
                #regex that will pull out the function being called
                match = re.search(r'%lo\(([^)]+)\)', full_file[call_index-searchback])
                if match:
                    return match.group(1)
            else:
                match = re.search(r'\((.*)\)',full_file[call_index-searchback]) #use anything in parentheses
                if match:
                    return match.group(1)
        #Special cases that would otherwise fail. Disable any one of these to see where it's needed.
        if "8016B3E4" in callreg_setline: #weird case in one function with compiler optimizing two calls into one
            return "PlaySfx"
        if "0xB8($a2)" in callreg_setline or "-0x20($s0)" in callreg_setline:
            return "UnknownEntityFunction"
        if "0x28($s0)" in callreg_setline or "-0xC($s0)" in callreg_setline:
            return "pfnEntityUpdate"
        if any(x in callreg_setline for x in ["80015840", "80015E74","8001923C"]): #weird sdk stuff
            return "UnknownSDKFunction"
        print("made it out the bottom")
        print(callreg_setline)
    print(call_line)
    print(full_file[3])
    print("Failed to return a value")

def get_g_api_funcs():
    funclist = []
    curr_struct_lines = []
    with open('../include/game.h') as f:
        lines = f.readlines()
    for line in lines:
        if "typedef struct" in line:
            curr_struct_lines = [line]
        elif len(curr_struct_lines): #We're in a struct and actively have added lines to it. This one is next.
            curr_struct_lines.append(line)
            if line[0] == "}" and len(curr_struct_lines): #We're at the end of the struct and it's all loaded into curr_struct_lines.
                structname = line.split(";")[0][2:] #Grab the name
                #If this is the struct we're looking for, process it. Otherwise forget the struct.
                if structname == "GameApi":
                    for line in curr_struct_lines:
                        match = re.search(r"^\s*\/\* ([a-fA-F0-9]+) \*\/\s+[^\s]+\s+\(?\*?([^\s\;\)]+)",line)
                        if match:
                            function_name = match.group(2)
                            funclist.append("g_api_" + function_name)
                if structname == 'Overlay':
                    for line in curr_struct_lines:
                        match = re.search(r"^\s*\/\* ([a-fA-F0-9]+) \*\/\s+[^\s]+\s+\(?\*?([^\s\;\)]+)",line)
                        if match:
                            function_name = match.group(2)
                            funclist.append("g_api_o_" + function_name)
                curr_struct_lines = [] #Empty it so we go back to just piping lines to output file
    return funclist
def get_sdk_funcs():
    functions = []
    #weird thing where setjmp is commented out in libc.h; we add it manually here for now.
    functions.append('setjmp')
    for sdkfile in ['libgpu.h','libc.h','libcd.h','libapi.h','kernel.h']:
        with open(f'../include/psxsdk/{sdkfile}') as f:
            lines = f.readlines()
            for line in lines:
                match = re.search(r'(\w+)\(',line) #Find a word right before an open parenthesis - a function name.
                if match:
                    functions.append(match.group(1))
    return functions

def get_main_funcs():
    functions = []
    files = ['9C54.s','160B4.s']
    for file in files:
        with open(f'../asm/us/main/{file}') as f:
            lines = f.readlines()
            for line in lines:
                if "glabel" in line:
                    functions.append(line.split()[1])
    return functions
def get_all_funcnames():
    api = get_g_api_funcs()
    sdk = get_sdk_funcs()
    cfuncs = [s.stem for s in Path('../asm').rglob("*.s") if 'nonmatchings' in str(s)]
    #files in src/main are from sdk and remain as asm, not broken out into function files
    mainfuncs = get_main_funcs()
    return  api + sdk + cfuncs + mainfuncs
    
def build_call_tree():
    tree_dict = {}
    all_func_names = get_all_funcnames()
    tree_dict["SDK_FUNCS"] = ",".join(get_sdk_funcs())
    tree_dict["G_API_FUNCS"] = ",".join(get_g_api_funcs())
    print("Functions loaded.")
    print(f"Function count: {len(all_func_names)}")
    print("Building call trees...")
    counter = 0
    for path in Path('../asm').rglob("*.s"):
        f = str(path)
        if 'mad' in f: #Skip mad for now, it has weird symbols
            continue
        if not 'nonmatchings' in f:
            continue
        with open(f) as opened_f:
            counter += 1
            filelines = opened_f.read().split('\n')
            foundfuncs = []
            for i,line in enumerate(filelines):
                if 'jal' in line:
                    funcname = handle_jal_call(filelines,i,all_func_names)
                    if funcname is None:
                        print(counter)
                    if funcname not in foundfuncs:
                        foundfuncs.append(funcname)
            
            tree_dict[path.stem] = ",".join(foundfuncs)
    return tree_dict

def get_all_c_files(src_dir):
    c_files_list = list()
    for root, dirs, files in os.walk(src_dir):
        for f in files:
            if f.endswith(".c"):
                c_files_list.append(os.path.join(root, f))
    return c_files_list

class NonMatchingFunc(object):
    def __init__(self, nonmatching_path):
        split = nonmatching_path.split("/")

        self.asm_path = nonmatching_path
        self.name = os.path.splitext(os.path.basename(nonmatching_path))[0]
        self.overlay_name = split[split.index("nonmatchings") - 1]
        self.text_offset = split[split.index("nonmatchings") + 1]

        assumed_path = f"/{self.overlay_name}/{self.text_offset}.c"
        c_paths = [src for src in src_files if src.endswith(assumed_path)]
        assert len(c_paths) == 1
        self.src_path = c_paths[0]

def get_nonmatching_functions(base_path, func_name) -> list:
    function_list = list()
    for root, dirs, files in os.walk(base_path):
        if "/nonmatchings/" in root:
            for f in files:
                if f == f"{func_name}.s":
                    full_path = os.path.join(root, f)
                    function = NonMatchingFunc(full_path)
                    function_list.append(function)
    if len(function_list) > 1:
        print(f'Found multiple functions named {func_name}, showing one')
    elif len(function_list) == 0:
        print('Function not found')
        print(func_name)
        kill=1/0
    return function_list[0]
def get_function_name(code):
    # match a function with a return type, name, parameters, and opening curly brace
    pattern = r'\w+\s+(\w+)\([^)]*\)\s*{'
    match = re.match(pattern, code)
    return match.group(1) if match is not None else None

def is_decompiled(srcfile,fname):
    with open(srcfile) as f:
        srclines = f.readlines()
    for line in srclines:
        if fname in line and 'INCLUDE_ASM' in line:
            return False
    return True

def analyze_function(fname,tree):
    
    foundfunc = get_nonmatching_functions('../asm', fname)
    decomp_done = str(is_decompiled(foundfunc.src_path,fname))
    if MODE == 'GRAPHICAL':
        graph = graphviz.Digraph(format='png')
        graph_colors = {'N/A':'lightblue','True':'green','False':'red'}
        graph.node(fname,style='filled',fillcolor=graph_colors[decomp_done])
    if MODE == 'CMDLINE':
        print(f"Analyzing {fname}; Decompiled: {decomp_done}")
        print(f"Functions called:")
    #Look through our asm file, and see who else we call.
    if len(tree[fname]) == 1:
        print("No functions called.")
    else:
        for item in tree[fname]:
            if item in tree['SDK_FUNCS'] or item in tree['G_API_FUNCS']:
                decomp_done = 'N/A'
            else:
                item_as_func = get_nonmatching_functions('../asm',item)
                decomp_done = str(is_decompiled(item_as_func.src_path,item))
            if MODE == 'GRAPHICAL':
                graph.node(item,style='filled',fillcolor=graph_colors[decomp_done])
                graph.edge(fname,item)
            if MODE == 'CMDLINE':
                print(f'{item}; Decompiled: {decomp_done}')
    if MODE == 'CMDLINE':
        print(f'\nFunctions which call this:')
    for key,value in tree.items():
        if fname in value:
            key_as_func = get_nonmatching_functions('../asm',key)
            decomp_done = str(is_decompiled(key_as_func.src_path,key))
            if MODE == 'GRAPHICAL':
                graph.node(key,style='filled',fillcolor=graph_colors[decomp_done])
                graph.edge(key,fname)
            if MODE == 'CMDLINE':
                print(f'{key}; Decompiled: {decomp_done}')
    if MODE == 'GRAPHICAL':
        imgbytes = graph.pipe()
        img = Image.open(io.BytesIO(imgbytes))
        img.show()

call_tree_filename = 'sotn_calltree.txt'
src_files = get_all_c_files('../src')

if __name__ == "__main__":
    tree = {}
    #Check if tree needs to be built.
    if not os.path.exists(call_tree_filename):
        #This is an arbitrary file for a decompiled function. It only exists after a force extract.
        if not os.path.exists('../asm/us/main/nonmatchings/5A38/ResetCallback.s'):
            print("Need to run `make force_extract` to build call tree!")
        tree = build_call_tree()
        with open(call_tree_filename,'w') as f:
            for key,value in tree.items():
                f.write(key + ":" +  value + "\n")
    with open(call_tree_filename) as f:
        treelines = f.readlines()
    for line in treelines:
       fname,calls = line[:-1].split(":")
       tree[fname] = calls.split(",")

    if len(sys.argv) == 1:
        MODE = 'GRAPHICAL'
    elif len(sys.argv) == 2:
        MODE = 'CMDLINE'
    else:
        print("Too many arguments!")
    if MODE == 'GRAPHICAL':
        while(True):
            startfunc = input("Give function to analyze:\n")
            analyze_function(startfunc,tree)
    if MODE == 'CMDLINE':
        analyze_function(sys.argv[1],tree)

