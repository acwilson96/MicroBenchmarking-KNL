## Login
ssh -l {username} login.archer.ac.uk
ssh knl-login

## Copy Files
scp ./{file}.tar.gz {username}@login.archer.ac.uk:

scp {file}.tar.gz knl-login:/work/knl-users/$USER/

## File Locations
/work/knl-users/$USER

## Compiling
module swap PrgEnv-cray PrgEnv-intel


## Submitting Jobs
 * qsub {batch script}.sh

{batch script}.sh should be of form:

```
#!/bin/bash
#PBS -N {Job Name}
#PBS -l select=1:aoe=quad_100
#PBS -l walltime=00:05:00
#PBS -A {budget account}

cd $PBS_O_WORKDIR
aprun -n {number of KNL cores to use} -d {number of threads} ./{application executable}
```
### Tracking Jobs

```unix
qstat -a        # Detailed job statuses.
qstat -f        # Detailed job list.
qstat -u $USER  # Jobs for current user only.
qstat -T        # Shows estimated start time of jobs.
```

## To configure memory modes
```
 #PBS -l select={number of KNL nodes to use}:aoe={L2 Cache Config}_{MCDRAM Config}
 ```
 ### Where:
  * __L2 Cache Config__ = [ a2a | quad | snc4 ]
  * __MCDRAM Config__   = [ 0 | 25 | 50 | 100 ] -  _s.t. the value is the % of memory used as cache._