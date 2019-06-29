oj_server:oj_server.cc   oj_model.hpp oj_view.hpp
	g++ oj_server.cc -o oj_server -std=c++11 -lpthread -ljsoncpp -lctemplate \
		-I ~/third_part/include -L ~/third_part/lib `mysql_config --cflags --libs`


compile_sever:compile_sever.cpp compile.hpp
	g++ compile_sever.cpp -o compile_sever -std=c++11 -lpthread -ljsoncpp

test:test.cc
	g++ test.cc -o test -std=c++11 -lpthread -ljsoncpp

.PHONY:clean
clean:
	rm test compile_sever

