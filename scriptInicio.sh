#!/usr/bin/sh
cd /home/utnso

git clone https://github.com/sisoputnfrba/tp-2017-1c-Dirty-Cow

export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/home/utnso/tp-2017-1c-Dirty-Cow/commons/Debug:/home/utnso/tp-2017-1c-Dirty-Cow/parser/Debug

cd /home/utnso/tp-2017-1c-Dirty-Cow/commons/Debug && make all
cd /home/utnso/tp-2017-1c-Dirty-Cow/parser/Debug && make all
cd /home/utnso/tp-2017-1c-Dirty-Cow/kernel/Debug && make all
cd /home/utnso/tp-2017-1c-Dirty-Cow/cpu/Debug && make all
cd /home/utnso/tp-2017-1c-Dirty-Cow/memoria/Debug && make all
cd /home/utnso/tp-2017-1c-Dirty-Cow/filesystem/Debug && make all
cd /home/utnso/tp-2017-1c-Dirty-Cow/consola/Debug && make all


