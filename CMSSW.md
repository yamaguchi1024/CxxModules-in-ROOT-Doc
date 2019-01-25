This page will be about Modules support for CMSSW in general.

* Useful words
CMSSW - CMS SoftWare.
scram - CMSSW build system. Equivalent to cmake

* Useful Links
https://github.com/cms-sw
https://cmssdt.cern.ch/SDT/

* Graphs
[](https://cmssdt.cern.ch/SDT/jenkins-artifacts/ib-dqm-tests/CMSSW_10_5_CXXMODULE_X_2019-01-08-2300/slc7_amd64_gcc700/150/memory_25202.0/mbGraph.html#?profile=performance.json&pid=_sum)

### How to build CMSSW
This the most challenging part out of everything. You need to expect at least melting one week on this.

1. Get a CMS account
In order to login to cmsdev10 to cmsdev15 by `ssh usename@cmsdev15.cern.ch`

2. Create a directory in AFS or some working directory, it'll take more than 50 GB

3. git clone
```
git clone https://github.com/cms-sw/cmsdist/
git clone https://github.com/cms-sw/pkgtools
```

4. Modify root.spec and root-toolfile.spec
```
cd cmsdist
```
Those spec files are the configuration of CMake, basically. Add -Druntime_cxxmodule=On or do whatever you want.

5. Go to singularity
```
singularity shell -B /afs -B /cvmfs -B /build -B /afs/cern.ch:/afs/cern.ch docker://cmssw/cc7:latest
```
If it doesn't work, modify -B randomly and it will work.

6. Build cmssw-tool-conf (external packages)
```
./pkgtools/cmsBuild -a slc7_amd64_gcc700 --repo cms.week0 -c ./cmsdist -i <directory name that you want to create> -j 16 build cmssw-tool-conf &
```
First of all, `slc7_amd64_gcc700` is called "architecture". `--repo cms.week0` is a CMSSW release from which this build take external packages from. It can be `--repo cms.week1` depending on which production week you're in.
If you look at
```
[yuka@cmsdev15 CMSSW_10_5_ROOT6_X_2019-01-20-0000]$ ls /cvmfs/cms-ib.cern.ch/nweek-025
nweek-02559/ nweek-02560/
```
the larger number is this week's relase. So in this case, nweek-02560. If the laste degit is even, `--repo cms.week<number>` is 0. If odd, it's 1 :)

Also, you can build packages with "reference". When fetching external packages, this "reference" allows to create a symlink under packages' installed directory to /cvmfs/cms-ib.cern.ch/nweek-something/slc7_amd64_gcc700/external/. This is how external packages are distributed. Modules had an issue with this, which is now fixed. To enable this, you can add `--reference=/cvmfs/cms-ib.cern.ch/nweek-02555` to cmsBuild execution.

You can also use source from current directory, not by fetching. You need to clone ROOT and add `--source root:Source=root-6.17.01`.

7. Build CMSSW
Successfully build cmssw-tool-conf? Then we can finally build CMSSW!

```
source /cvmfs/cms.cern.ch/cmsset_default.sh
scram -a slc7_amd64_gcc700 list CMSSW_10_5_CXXMODULE_X_`
Pick one IB from the list
scram -a slc7_amd64_gcc700 p <IB that you picked>
cd CMSSW_IB_YOU PICKED
mv  config/toolbox/slc7_amd64_gcc700/tools/selected config/toolbox/slc7_amd64_gcc700/tools/selected.original
cp ../slc7_amd64_gcc700/cms/cmssw-tool-conf/45.0-cms/tools/selected config/toolbox/slc7_amd64_gcc700/tools/selected -r
scram setup
cmsenv
git cms-addpkg <Package that you want to build>
scram b -j 10
Wait for 3 hours
```

git cms-addpkg is a command to clone CMSSW packages from github. If you don't know which packages to build, just do like `scram b echo_root_interface_USED_BY | tr ' ' '\n' | grep '^self/' | cut -d/ -f2-3 | sort | uniq | xargs git cms-addpkg` which clones all packages that has ROOT has their dependency. DataFormats is a main package in CMSSW.
