      program hello
      implicit none
      include 'mpif.h'
      integer(kind=MPI_INTEGER_KIND) ierror
      call MPI_INIT(ierror)
      call MPI_FINALIZE(ierror)
      end program
