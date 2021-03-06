/*  -----------------------------------------------------
 *   Isotropic elastic forward modelling for all shots
 *
 *
 *   D. Koehn
 *   Kiel, 21.10.2017
 *
 *  -----------------------------------------------------*/

#include "fd.h"
#include "fd3d.h"	/* general include file for program FD3D */
#include "fd3del.h"	/* include file for elastic version */
#include "fd3diso.h"	/* include file for isotropic version */
#include "ISO_struct.h"	/* data structures for isotropic elastic forward modelling */

void forward_ISO(struct wave *wave, struct pmls *pmls, struct mat *mat, struct geom *geom, struct mpi *mpi, 
                 struct seis *seis, struct acq *acq, struct times *times, int ns){

        /* global variables */
	extern int RUN_MULTIPLE_SHOTS, NSRC, SHOTNO, SHOTNO_LOC, SRCSIGNAL, MYID, NT;
	extern int NSHOT1, NSHOT2;
	extern char MFILE[STRING_SIZE];
	extern FILE *FP;

        /* local variables */
	int nshots, ishot, ishot1, ishot2;	

        /* read source positions and source parameters from SOURCE_FILE and assign positions to local grids */
        (*acq).srcpos     = sources((*geom).xg, (*geom).yg, (*geom).zg, (*geom).xpg, (*geom).ypg, (*geom).zpg);
	(*acq).srcpos_loc = splitsrc((*acq).srcpos);

	if (RUN_MULTIPLE_SHOTS) nshots=NSRC; else nshots=1;

	ishot1 = 1;
	ishot2 = nshots;

	/* loop over shots */
	for (ishot=NSHOT1;ishot<NSHOT2;ishot++){

    		SHOTNO = ishot;
		shotno_glob2loc(acq);


        	if(!MYID){
		   fprintf(FP,"\n==================================================================================\n");
		   fprintf(FP,"\n *****  Starting simulation (forward model) for shot %d of %d  ********** \n",ishot,nshots);
		   fprintf(FP,"\n==================================================================================\n\n");
		}

    		/* calculate wavelet for each source point */
    		if (SHOTNO_LOC) (*acq).signals = wavelet((*acq).srcpos_loc);

    		/* output source signal e.g. for cross-correlation or comparison with analytical solutions */
    		if ((SRCSIGNAL<6)&&(SHOTNO_LOC)){

	    		char  source_signal_file[STRING_SIZE];
	    		sprintf(source_signal_file,"%s_source_signal.%d.su", MFILE, MYID);
	    		fprintf(stdout,"\n PE %d outputs source time function in SU format to %s \n ", MYID, source_signal_file);
	    		output_source_signal(fopen(source_signal_file,"w"), (*acq).signals, (*acq).srcpos_loc, NT, 1, (*geom).xg, 
					     (*geom).yg, (*geom).zg, (*geom).xpg, (*geom).ypg, (*geom).zpg);
    		}

    		MPI_Barrier(MPI_COMM_WORLD);

		/* reset wavefields before shot modelling */
		reset_wavefields_iso(wave, pmls, seis, ns);

    		/* Isotropic elastic forward modelling */
    		forward_shot_ISO(wave, pmls, mat, geom, mpi, seis, acq, times, ns);

	} /* end of loop over shots */

}
