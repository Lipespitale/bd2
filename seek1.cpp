#include "B+PrimaryIndex.hpp"

int main(int argc, char *argv[]){
    seek1(HASH_FILE_NAME, PRIM_INDEX_FILE_NAME, atoi(argv[1]));
	return 0;
}