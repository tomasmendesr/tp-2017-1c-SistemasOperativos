#!/usr/bin/sh
git clone https://github.com/sisoputnfrba/tp-2017-1c-Dirty-Cow
export LD_LIBRARY_PATH=/home/utnso/tp-2017-1c-Dirty-Cow/commons/Debug:/home/utnso/tp-2017-1c-Dirty-Cow/parser/Debug


cd tp-2017-1c-Dirty-Cow/ && sudo chmod -R 777 *
cd commons/Debug && make clean && make
cd ../../parser/Debug && make clean && make 
cd ../../fileSystem/Debug && make clean && make 
cd ../../memoria/Debug && make clean && make 
cd ../../consola/Debug && make clean && make 
cd ../../cpu/Debug && make clean && make 
cd ../../kernel/Debug && make clean && make 