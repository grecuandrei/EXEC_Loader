/*
 * Loader Implementation
 *
 * 2018, Operating Systems
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>

#include "exec_parser.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

static so_exec_t *exec;
static int page_size;
static struct sigaction old_action;
static char *p;
static int fd;

typedef struct mapped_data {
	int *mapped;
} mapped_data_t;

static void segv_handler(int signum, siginfo_t *info, void *context)
{
	char *addr = (char *)info->si_addr;
	int i, valid = 0, page, ret;
	char *buffer = calloc(page_size, sizeof(char)); // buffer for read data

	// handle the signal if it is not a SIGSEGV
	if(signum != SIGSEGV) {
		old_action.sa_sigaction(signum, info, context);
		return;
	}

	for (i = 0; i < exec->segments_no; i++) {
		so_seg_t curr_seg = exec->segments[i];

		if ((uintptr_t)addr >= curr_seg.vaddr && (uintptr_t)addr <= (curr_seg.vaddr + curr_seg.mem_size)) {
			// valid adress
			valid = 1;
			page = floor((uintptr_t)(((uintptr_t)addr - (uintptr_t)curr_seg.vaddr) / page_size));
			mapped_data_t *extra_info = (mapped_data_t *)curr_seg.data;

			if (extra_info->mapped[page] == 0) {
				// mmap this page
				unsigned int page_start_offset_for_file = curr_seg.offset + page * page_size;
				unsigned int page_size_of_data = MIN(page_size, curr_seg.file_size - page * page_size);
				void *mmap_start = (void *)curr_seg.vaddr + page * page_size;

				if (page * page_size <= curr_seg.file_size) {
					// mapping
					ret = lseek(fd, page_start_offset_for_file, SEEK_SET);
					if (ret < 0) {
						perror("lseek error");
						exit(-1);
					}
					p = mmap(mmap_start, page_size, PROT_WRITE, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
					if (p == MAP_FAILED) {
						perror("mmap error");
						exit(-1);
					}
					ret = read(fd, buffer, page_size_of_data);
					if (ret < 0) {
						perror("read error");
						exit(-1);
					}
					memcpy(p, buffer, page_size);
				} else {
					// zeroing all page
					p = mmap(mmap_start, page_size, PROT_WRITE, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
					if (p == MAP_FAILED) {
						perror("mmap error");
						exit(-1);
					}
					memcpy(p, buffer, page_size);
				}
				mprotect(p, page_size, curr_seg.perm);
				extra_info->mapped[page] = 1;
			} else if (extra_info->mapped[page] == 1) {
				// return SIGSEGV
				old_action.sa_sigaction(signum, info, context);
				free(buffer);
				return;
			}
		}
	}
	if (valid == 0) {
		// no valid pages in any segment
		old_action.sa_sigaction(signum, info, context);
		free(buffer);
		return;
	}
	free(buffer);
}

int so_init_loader(void)
{
	/* TODO: initialize on-demand loader */
	// set up handler
	struct sigaction action;
	int rc;

	memset(&action, 0, sizeof(action));
	action.sa_sigaction = segv_handler;
	sigemptyset(&action.sa_mask);
	sigaddset(&action.sa_mask, SIGSEGV);
	action.sa_flags = SA_SIGINFO;

	rc = sigaction(SIGSEGV, &action, &old_action);
	if (rc == -1) {
		perror("sigaction");
		exit(-1);
	}

	return 0;
}

int so_execute(char *path, char *argv[])
{
	int i, size;

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		perror("open file error");
		exit(-1);
	}
	exec = so_parse_exec(path);
	if (!exec)
		return -1;
	page_size = getpagesize();
	// for every segment set up an array in data
	for (i = 0; i < exec->segments_no; i++) {
		mapped_data_t *data = calloc(1, sizeof(mapped_data_t));

		if (data == NULL)
			return -1;
		size = exec->segments[i].mem_size / page_size;
		data->mapped = calloc(MAX(size, 1), sizeof(int));
		if (data->mapped == NULL)
			return -1;
		exec->segments[i].data = calloc(MAX(size, 1) + 1, sizeof(int));
		if (exec->segments[i].data == NULL)
			return -1;
		memcpy(exec->segments[i].data, data, sizeof(*data));
	}

	so_start_exec(exec, argv);
	close(fd);

	return -1;
}
