do_so:
	gcc -shared -fPIC -o libown1.so lib1.c
	gcc -shared -fPIC -o libown2.so lib2.c

comp_stat:
	gcc ./example_static.c -L. -lown2 -Wl,-rpath,. -o example_static

comp_dyn:
	gcc ./example_dynamic.c -ldl -o example_dynamic