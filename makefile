ORACLE_HOME=/home/oracle/app/oracle/product/11.2.0/dbhome_1
libyace:
	gcc -g -shared -o libyace.so -fPIC -I$(ORACLE_HOME)/rdbms/public -Wall libyace.c
