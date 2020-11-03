#include <iostream>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include <libpmem.h>
#include <errno.h>

#define DEFAULT_SIZE 4096
using namespace std;

int main(int argc, char *argv[]) {
	int fd;
	char *pmemaddr;
	size_t requested, mapped_len;
	int is_pmem;

	if (argc != 2 && argc != 3) {
		std::cerr << "Usage:\n\t./nvm_test <pmem file> [size]\n";
		return 1;
	}

	requested = (argc == 3) ? atol(argv[2]) : DEFAULT_SIZE;

	/* create a pmem file and memory map it
	 * Do note that if the provided file is /dev/daxX.Y (dax device, rather than dax file),
	 * then the requested size must be same as the size of the device itself. Otherwise,
	 * pmem_map_file function will return NULL.
	 */
	if ((pmemaddr = (char *) pmem_map_file(argv[1], requested, PMEM_FILE_CREATE, 0666, &mapped_len, &is_pmem)) == NULL) {
		perror("pmem_map_file");
		exit(1);
	}

	/* store a string to the persistent memory */
	strcpy(pmemaddr, "hello, persistent memory");

	/* flush above strcpy to persistence */
	if (is_pmem) {
		std::cout << "It is a persistent memory\n";
		pmem_persist(pmemaddr, mapped_len);
	} else {
		std::cout << "It is not a persistent memory\n";
		pmem_msync(pmemaddr, mapped_len);
	}

	pmem_unmap(pmemaddr, mapped_len);

    return 0;
}
