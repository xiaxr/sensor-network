project('xiaxr_hub', 'cpp')

add_project_arguments('-I./rpi', '-I./RF24X','-I./utility', 
    '-Wall', 
    '-fPIC', 
#    '-g',
    '-Ofast',
    '-mfpu=vfp',
    '-mfloat-abi=hard',
    '-march=armv7-a',
    '-mtune=arm1176jzf-s',
    '-std=c++20',
    language : 'cpp')

# add_project_link_arguments('-g', language: 'cpp')

cpp_compiler = meson.get_compiler('cpp')

boost_libs = dependency('boost') 
pthread_lib = dependency('threads')
cap_lib = dependency('libcap')
fs_lib =  cpp_compiler.find_library('stdc++fs')

executable('xiaxr_hub', 
    'main.cpp', 'gateway.cpp', 'measurement.cpp', 'device.cpp', 'config.cpp', 
    'utility/delay.cpp',
    'RF24NetworkX/RF24Network.cpp', 
    'RF24X/RF24.cpp', 
    'rpi/bcm2835.cpp',
    'rpi/interrupt.cpp',
    'rpi/spi.cpp',
    dependencies: [boost_libs, fs_lib, cap_lib, pthread_lib])
