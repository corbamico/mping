project('mping', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++20'])

compiler = meson.get_compiler('cpp')
cxx      = compiler.get_id()

if cxx=='clang'
  add_project_link_arguments('-lstdc++', language: 'cpp')
endif

if cxx=='msvc'
  # for vs2022 env.
  incdir = include_directories('C:/DevSoft/vcpkg/vcpkg/installed/x64-windows/include')
  deps = []
elif ( cxx == 'gcc' or cxx == 'clang' ) and host_machine.system() == 'windows'
  # for gcc and clang  on msys2 env.
  incdir = []
  deps = [compiler.find_library('ws2_32', required: true)]
elif  ( cxx == 'gcc' or cxx == 'clang' ) and host_machine.system() != 'windows'
  # for gcc and clang on linux env.
  incdir = []
  deps = []
endif 



executable('mping',
           'src/main.cpp',
           include_directories: incdir,
           dependencies: deps,
           )


