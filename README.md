<img src="https://regmedia.co.uk/2016/10/20/dirty_cow.jpg?x=648&y=348&crop=1">
<h2>branch Version-1.0:</h2>
<p>Todo andando para el 1er Checkpoint, se crearon protocolos y kernel soporta multiples conexiones de consolas y cpu. La memoria funciona como servidor multi-threading por cada conexion entrante. Se dividio el kernel en 2 hilos un planificador de largo plazo y uno de corto plazo para manejar las multiples conexion servidor/consola (Chequear esto!)</p>

export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/home/utnso/tp-2017-1c-Dirty-Cow/commons/Debug:/home/utnso/tp-2017-1c-Dirty-Cow/parser/Debug