import os
import re
import subprocess
import ycm_core

myFlags = [
    '-Wall',
    '-Wextra',
    '-Wfloat-equal',
    '-Wundef',
    '-Wshadow',
    '-Wpointer-arith',
    '-Wstrict-prototypes',
    '-Wstrict-overflow=5',
    '-Wwrite-strings',
    '-Waggregate-return',
    '-Wcast-qual',
    '-Wswitch-default',
    '-Wswitch-enum',
    '-Wconversion',
    '-Wunreachable-code',
    '-Winit-self',
    '-Werror-implicit-function-declaration',
    '-Wmissing-prototypes',
    '-Wno-unused-result',
    '-std=c++14',
    '-stdlib=libc++',
    '-x', 'c++',
    '-I', './src/include',
    '-I', './build/metis/include',
    '-I', './deps/KaHIP/deploy',
    '-I', './src/main',
    '-I', './third-party/openssl-1.0.1i/include',
    '-I', './deps/args'
]

database = None

SOURCE_EXTENSIONS = ['.cpp']

def LoadSystemIncludes():
    regex = re.compile(r'(?:\#include \<...\> search starts here\:)(?P<list>.*?)(?:End of search list)', re.DOTALL)
    process = subprocess.Popen(['clang', '-v', '-E', '-x', 'c++', '-'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    process_out, process_err = process.communicate('')
    output = str(process_out + process_err, 'utf-8')
    includes = []
    for p in re.search(regex, output).group('list').split('\n'):
        p = p.strip()
        if len(p) > 0 and p.find('(framework directory)') < 0:
            includes.append('-isystem')
            includes.append(p)
    return includes

def MakeRelativePathsInFlagsAbsolute( flags, working_directory ):
    if not working_directory:
        return list( flags )
    new_flags = []
    make_next_absolute = False
    path_flags = [ '-isystem', '-I', '-iquote', '--sysroot=' ]
    for flag in flags:
        new_flag = flag

        if make_next_absolute:
            make_next_absolute = False
            if not flag.startswith( '/' ):
                new_flag = os.path.join( working_directory, flag )

        for path_flag in path_flags:
            if flag == path_flag:
                make_next_absolute = True
                break

            if flag.startswith( path_flag ):
                path = flag[ len( path_flag ): ]
                new_flag = path_flag + os.path.join( working_directory, path )
                break

        if new_flag:
            new_flags.append( new_flag )
    return new_flags


def IsHeaderFile( filename ):
    extension = os.path.splitext( filename )[ 1 ]
    return extension in [ '.hpp', '.h' ]


def GetCompilationInfoForFile( filename ):
    # The compilation_commands.json file generated by CMake does not have entries
    # for header files. So we do our best by asking the db for flags for a
    # corresponding source file, if any. If one exists, the flags for that file
    # should be good enough.
    if IsHeaderFile( filename ):
        basename = os.path.splitext( filename )[ 0 ]
        for extension in SOURCE_EXTENSIONS:
            replacement_file = basename + extension
            if os.path.exists( replacement_file ):
                compilation_info = database.GetCompilationInfoForFile(
                        replacement_file )
                if compilation_info.compiler_flags_:
                    return compilation_info
        return None
    return database.GetCompilationInfoForFile( filename )


def FlagsForFile( filename, **kwargs ):
    final_flags = myFlags + LoadSystemIncludes()
    if database:
        # Bear in mind that compilation_info.compiler_flags_ does NOT return a
        # python list, but a "list-like" StringVec object
        compilation_info = GetCompilationInfoForFile( filename )
        if not compilation_info:
            return None

        flags = MakeRelativePathsInFlagsAbsolute(
                compilation_info.compiler_flags_,
                compilation_info.compiler_working_dir_ )

        final_flags += flags

    return {
            'flags': final_flags,
            'do_cache': True
            }

