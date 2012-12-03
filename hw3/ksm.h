//
// File created by Flavia Tovo as work for project 3
// of OS class on KAUST
// Fall 2012
//

// RETURN CODES
#define KSM_READWRITE 0
#define KSM_READ 1

// KSM info struct
struct ksminfo_t {
  uint ksmsz;               // The size of the shared memory
  int cpid;                 // PID of the creator
  int mpid;                 // PID of last modifier
  uint attached_nr;         // Number of attached processes
  uint atime;               // Last attack time
  uint dtime;               // Last detach time
  uint total_shrg_nr;       // Total number of existing shared regions
  uint total_shpg_nr;       // Total number of existing shared pages
};

// Added to defs.h
// int ksmget(char *, uint);
// int ksmattach(int, int);
// int ksmdetack(int);
// int ksminfo(int, struct ksminfo_t*);
// int ksmdelete(int);