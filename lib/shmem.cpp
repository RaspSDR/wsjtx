#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <semaphore.h>
#include <iostream>

// POSIX Shared Memory Manager Class
class Shmem {
public:
    Shmem() : shm_fd(-1), mem_ptr(nullptr), sem_ptr(nullptr) {}

    bool create(int nsize) {
        shm_fd = shm_open("/shm_example", O_CREAT | O_RDWR, 0666);
        if (shm_fd == -1) {
            perror("shm_open failed");
            return false;
        }
        if (ftruncate(shm_fd, nsize) == -1) {
            perror("ftruncate failed");
            close(shm_fd);
            return false;
        }

        mem_ptr = mmap(NULL, nsize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (mem_ptr == MAP_FAILED) {
            perror("mmap failed");
            close(shm_fd);
            return false;
        }

        // Create semaphore for synchronization
        sem_ptr = sem_open("/shm_semaphore", O_CREAT, 0666, 1);
        if (sem_ptr == SEM_FAILED) {
            perror("sem_open failed");
            munmap(mem_ptr, nsize);
            close(shm_fd);
            return false;
        }

        this->nsize = nsize;
        return true;
    }

    void setKey(const std::string& mykey) {
        key = mykey;
    }

    bool attach() {
        if (shm_fd == -1) {
            shm_fd = shm_open("/shm_example", O_RDWR, 0666);
            if (shm_fd == -1) {
                perror("shm_open failed");
                return false;
            }
        }

        mem_ptr = mmap(NULL, nsize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (mem_ptr == MAP_FAILED) {
            perror("mmap failed");
            close(shm_fd);
            return false;
        }

        sem_ptr = sem_open("/shm_semaphore", 0);
        if (sem_ptr == SEM_FAILED) {
            perror("sem_open failed");
            munmap(mem_ptr, nsize);
            close(shm_fd);
            return false;
        }

        return true;
    }

    int size() const {
        struct stat sb;
        if (fstat(shm_fd, &sb) == -1) {
            perror("fstat failed");
            return -1;
        }
        return sb.st_size;
    }

    void* data() {
        return reinterpret_cast<void*>(mem_ptr);
    }

    bool lock() {
        return sem_wait(sem_ptr) == 0;
    }

    bool unlock() {
        return sem_post(sem_ptr) == 0;
    }

    bool detach() {
        if (munmap(mem_ptr, nsize) == -1) {
            perror("munmap failed");
            return false;
        }
        close(shm_fd);
        if (sem_close(sem_ptr) == -1) {
            perror("sem_close failed");
            return false;
        }
        return true;
    }

    ~Shmem() {
        if (mem_ptr != nullptr) {
            munmap(mem_ptr, size());
        }
        if (shm_fd != -1) {
            close(shm_fd);
        }
        if (sem_ptr != nullptr) {
            sem_close(sem_ptr);
            sem_unlink("/shm_semaphore");
        }
    }

private:
    int shm_fd;
    int nsize;
    void* mem_ptr;
    sem_t* sem_ptr;
    std::string key;
};

static Shmem shmem;

struct jt9com;

// C wrappers for a QSharedMemory class instance
extern "C"
{
  bool shmem_create (int nsize) {return shmem.create(nsize);}
  void shmem_setkey (char * const mykey) {shmem.setKey(mykey);}
  bool shmem_attach () {return shmem.attach();}
  int shmem_size () {return static_cast<int> (shmem.size());}
  struct jt9com * shmem_address () {return reinterpret_cast<struct jt9com *>(shmem.data());}
  bool shmem_lock () {return shmem.lock();}
  bool shmem_unlock () {return shmem.unlock();}
  bool shmem_detach () {return shmem.detach();}
}
