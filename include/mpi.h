
#ifndef AISLINN_MPI_HEADER
#define AISLINN_MPI_HEADER

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/** Fake structures */
struct aislinn_request;
struct aislinn_status;

/* Public MPI types */
typedef int MPI_Request;
typedef int MPI_Comm;
typedef aislinn_status *MPI_Status;
typedef int MPI_Datatype;

/* Constants */
const MPI_Comm MPI_COMM_WORLD = 0;
const MPI_Datatype MPI_INT = 1;
#define MPI_STATUS_IGNORE ((MPI_Status*) 0)
#define MPI_STATUSES_IGNORE ((MPI_Status*) 0)
#define MPI_ANY_SOURCE -1

/* Public MPI Functions */
inline int MPI_Init(int *argc, char ***argv) {
	// Currently do nothing
	return 0;
}

inline int MPI_Finalize() {
	// Currently do nothing
	return 0;
}

int MPI_Comm_rank(MPI_Comm comm, int *rank);

int MPI_Isend(void *buf, int count, MPI_Datatype datatype, int dest,
    int tag, MPI_Comm comm, MPI_Request *request);

int MPI_Irecv(void *buf, int count, MPI_Datatype datatype,
        int source, int tag, MPI_Comm comm, MPI_Request *request);

int MPI_Wait(MPI_Request *request, MPI_Status *status);

int MPI_Waitall(int count, MPI_Request array_of_requests[],
               MPI_Status array_of_statuses[]);

int MPI_Test(MPI_Request *request, int *flag, MPI_Status *status);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // AISLINN_MPI_HEADER
