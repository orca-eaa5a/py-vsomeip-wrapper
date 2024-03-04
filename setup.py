from distutils.core import setup, Extension

vsomeipPy = Extension('vsomeip',
                    sources = ['vsomeip_python.cpp'],
                    libraries = ['vsomeip', 'vsomeip-cfg'],
                    library_dirs = ['/usr/local/lib/', '/home/orca/proj/vsomeip-3.4.9-r1/build'],
                    extra_compile_args=['-std=c++11'],
                    runtime_library_dirs=['/usr/local/lib/', '/home/orca/proj/vsomeip-3.4.9-r1/build'],
                    include_dirs=['/home/orca/proj/vsomeip-3.4.9-r1/interface'])

setup (name = 'vsomeipPy',
       version = '1.0',
       description = 'vsomeip crude Python bindings',
       ext_modules = [vsomeipPy])
