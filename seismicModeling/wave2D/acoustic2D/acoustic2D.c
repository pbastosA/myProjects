# include <time.h>
# include <math.h>
# include <stdio.h>
# include <stdlib.h>
# include "auxiliaries/functions.h"

int main(int argc, char **argv) 
{
    float total_time;
    time_t t_0, t_f;
    t_0 = time(NULL);

    int nx,nz,nt,nsrc,spread;
    int absLayer,nrecs,nshot; 
    float parB,dx,dz,dt; 

    readParameters(&nx,&nz,&nt,&dx,&dz,&dt,&absLayer,&nrecs,&nshot,&nsrc,&spread,argv[1]);

    int nxx = nx + 2*absLayer;
    int nzz = nz + 2*absLayer;

    float * vels   = (float *) malloc(2*sizeof(float));
    float * damp   = (float *) malloc(nxx*nzz*sizeof(float));     
    float * vp     = (float *) malloc(nxx*nzz*sizeof(float));  
    float * P_pas  = (float *) malloc(nxx*nzz*sizeof(float));   
    float * P_pre  = (float *) malloc(nxx*nzz*sizeof(float));  
    float * P_fut  = (float *) malloc(nxx*nzz*sizeof(float));  
    float * source = (float *) malloc(nsrc*sizeof(float));    

    char seismFile[100], snapsFile[100];
    float * seismogram = (float *) malloc(nt*spread*sizeof(float));
    float * snapshots  = (float *) malloc(nx*nz*sizeof(float));

    int *xsrc = (int *) malloc(nshot*sizeof(int));         
    int *xrec = (int *) malloc(nrecs*sizeof(int));      
    int *topo = (int *) malloc(nxx*sizeof(int));   

    importFloatVector(vp,nxx*nzz,argv[2]);
    importFloatVector(damp,nxx*nzz,argv[3]);
    importFloatVector(source,nsrc,argv[4]);

    importIntegerVector(xsrc,nshot,argv[5]);
    importIntegerVector(xrec,nrecs,argv[6]);

    vels = getVelocities(nxx,nzz,vp);

    ajustCoordinates(xrec,xsrc,topo,absLayer,nxx,nrecs,nshot);

    for (int i = 0; i < spread; i++) printf("%i\n",xrec[i]);

    for(int shot = 0; shot < 1; ++shot) 
    {        
        setWaveField(P_pas,P_pre,P_fut,nxx*nzz);

        sprintf(snapsFile,"results/snapshots/snaps_shot_%i.bin",shot+1);
        sprintf(seismFile,"results/seismograms/seism_shot_%i.bin",shot+1);   
                
        FILE * snap = fopen(snapsFile,"ab");
        for(int timePointer = 0; timePointer < nt; ++timePointer) 
        {
            modelingStatus(shot,timePointer,xsrc,nshot,xrec,spread,dx,dz,nt,vels,dt,nxx,nzz,absLayer);
            FDM_8E2T_acoustic2D(shot,timePointer,vp,P_pre,P_pas,P_fut,source,nsrc,topo,xsrc,nxx,nzz,dx,dz,dt);
            cerjanAbsorbingBoundaryCondition(P_pas,P_pre,P_fut,damp,nxx,nzz);
            getSeismograms(seismogram,P_pre,xrec,topo,spread,nxx,shot,timePointer);            
            getSnapshots(snap,snapshots,P_pre,vp,nxx,nzz,absLayer,timePointer,nt,100,5e-8);
            waveFieldUpdate(P_pas,P_pre,P_fut,nxx*nzz);
        }
        exportVector(seismogram,nt*spread,seismFile);
        fclose(snap);
    }

    t_f = time(NULL);                
    total_time = difftime(t_f, t_0); 
    printf("\nExecution time: \033[31m%.0fs\n\033[m", total_time);

    return 0;
}
