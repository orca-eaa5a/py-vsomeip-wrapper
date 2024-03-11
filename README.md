# py-vsomeip-wrapper
### vsomeip wrapper of python, implemented with Python C/C++ extending

## Denepdencies
- [https://github.com/GENIVI/vsomeip](https://github.com/GENIVI/vsomeip)

## Build & Install
1. before launch setup.py, you should build vsomeip first
2. edit setup.py
```
from  distutils.core  import  setup, Extension

vsomeipPy  =  Extension('vsomeip',
sources  = ['vsomeip_python.cpp'],
libraries  = ['vsomeip3', 'vsomeip3-cfg'],

library_dirs  = ['/usr/local/lib/', '{$vsomeip-build-path}/build/'],
runtime_library_dirs=['/usr/local/lib/', '{$vsomeip-build-path}/build/'],
extra_compile_args=['-std=c++11'],
include_dirs=['{$vsomeip-build-path}/interface'])

setup (
    name  =  'vsomeipPy',
    version  =  '1.0',
    description  =  'vsomeip crude Python bindings',
    ext_modules  = [vsomeipPy]
)	
```
3. add **LD_LIBRARY_PATH** environment variable (or shell variable)
```
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:{$vsomeip-build-path/build}
```
4. launch setup.py with build option
```
python setup.py build
```
5. After build successfully finished, launch setup.py with install option
 ```
python setup.py install
```
## Docker
The version of vsomeip in docker image is 
```
docker build -t .
```
## Reference
- This project based on [https://github.com/granquet/vsomeipPy].
- Python script examples are written by refering [https://github.com/COVESA/vsomeip/wiki/vsomeip-in-10-minutes].