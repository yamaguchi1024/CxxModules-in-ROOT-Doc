This page is dedicated to Modules support for CMSSW in general.

For CMSSW, ROOT is just one of many external packages. However it relys on dictionary generation with genreflex, and main binary of CMSSW (cmsRun) is using ROOT runtime behind the scene.

### How to build CMSSW
This the most challenging part out of everything. You need to expect melting at least one week on this.

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
  137  source slc7_amd64_gcc700/lcg/root/6.17.01-cms/bin/thisroot.sh
  138  source slc7_amd64_gcc700/lcg/root/6.17.01-cms/etc/profile.d/init.sh
root -l
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
scram b -k -j 8  runtests // Run tests
```

git cms-addpkg is a command to clone CMSSW packages from github. If you don't know which packages to build, just do like `scram b echo_root_interface_USED_BY | tr ' ' '\n' | grep '^self/' | cut -d/ -f2-3 | sort | uniq | xargs git cms-addpkg` which clones all packages that has ROOT has their dependency. DataFormats is a main package in CMSSW.


### Genreflex in CMSSW

```
/build/yuka/genreflex/build-cmssw-tool-conf/slc7_amd64_gcc700/lcg/root/6.17.01-efjacf/bin/genreflex src/DataFormats/TrackReco/src/classes.h -s src/DataFormats/TrackReco/src/classes_def.xml -o tmp/slc7_amd64_gcc700/src/DataFormats/TrackReco/src/DataFormatsTrackReco/a/DataFormatsTrackReco_xr.cc --deep -cxxmodule --fail_on_warnings --rootmap=DataFormatsTrackReco_xr.rootmap --rootmap-lib=libDataFormatsTrackReco.so -m DataFormatsTrackCandidate_xr_rdict.pcm -m DataFormatsTrajectorySeed_xr_rdict.pcm -m DataFormatsTrackingRecHit_xr_rdict.pcm -m DataFormatsSiStripDetId_xr_rdict.pcm -m DataFormatsBeamSpot_xr_rdict.pcm -m DataFormatsGeometrySurface_xr_rdict.pcm -m DataFormatsTrackerCommon_xr_rdict.pcm -m DataFormatsCLHEP_xr_rdict.pcm -m DataFormatsGeometryVector_xr_rdict.pcm -m DataFormatsMuonDetId_xr_rdict.pcm -m DataFormatsSiPixelDetId_xr_rdict.pcm -m DataFormatsDetId_xr_rdict.pcm -m DataFormatsMath_xr_rdict.pcm -m DataFormatsSiStripCluster_xr_rdict.pcm -m DataFormatsCommon_xr_rdict.pcm -m DataFormatsProvenance_xr_rdict.pcm -m FWCoreMessageLogger_xr_rdict.pcm -m DataFormatsStdDictionaries_xr_rdict.pcm -m DataFormatsStdDictionaries_x1r_rdict.pcm -m DataFormatsStdDictionaries_x2r_rdict.pcm -m DataFormatsStdDictionaries_x3r_rdict.pcm -m DataFormatsTrajectoryState_xr_rdict.pcm --capabilities=DataFormatsTrackReco_xi.cc -DCMS_DICT_IMPL -D_REENTRANT -DGNUSOURCE -D__STRICT_ANSI__ -DGNU_GCC -D_GNU_SOURCE -DTBB_USE_GLIBCXX_VERSION=70301 -DBOOST_SPIRIT_THREADSAFE -DPHOENIX_THREADSAFE -DCMSSW_GIT_HASH="CMSSW_10_5_ROOT6_X_2019-01-11-2300" -DPROJECT_NAME="CMSSW" -DPROJECT_VERSION="CMSSW_10_5_ROOT6_X_2019-01-11-2300" -I/build/yuka/genreflex/build-cmssw-tool-conf/CMSSW_10_5_ROOT6_X_2019-01-11-2300/src -I/cvmfs/cms-ib.cern.ch/week0/slc7_amd64_gcc700/cms/cmssw-patch/CMSSW_10_5_ROOT6_X_2019-01-11-2300/src -I/build/yuka/genreflex/build-cmssw-tool-conf/slc7_amd64_gcc700/lcg/root/6.17.01-efjacf/include -I/build/yuka/genreflex/build-cmssw-tool-conf/slc7_amd64_gcc700/external/pcre/8.37-omkpbe2/include -I/build/yuka/genreflex/build-cmssw-tool-conf/slc7_amd64_gcc700/external/boost/1.67.0/include -I/build/yuka/genreflex/build-cmssw-tool-conf/slc7_amd64_gcc700/external/bz2lib/1.0.6-omkpbe2/include -I/build/yuka/genreflex/build-cmssw-tool-conf/slc7_amd64_gcc700/external/clhep/2.4.0.0-gnimlf/include -I/build/yuka/genreflex/build-cmssw-tool-conf/slc7_amd64_gcc700/external/gsl/2.2.1-omkpbe2/include -I/build/yuka/genreflex/build-cmssw-tool-conf/slc7_amd64_gcc700/external/libuuid/2.22.2-omkpbe2/include -I/build/yuka/genreflex/build-cmssw-tool-conf/slc7_amd64_gcc700/external/tbb/2019_U3/include -I/build/yuka/genreflex/build-cmssw-tool-conf/slc7_amd64_gcc700/external/xz/5.2.2-omkpbe2/include -I/build/yuka/genreflex/build-cmssw-tool-conf/slc7_amd64_gcc700/external/zlib-x86_64/1.2.11-omkpbe2/include -I/build/yuka/genreflex/build-cmssw-tool-conf/slc7_amd64_gcc700/external/md5/1.0.0-omkpbe2/include -I/build/yuka/genreflex/build-cmssw-tool-conf/slc7_amd64_gcc700/external/tinyxml2/6.2.0/include -DCMSSW_REFLEX_DICT
```

```
 rootcling -v2 -f out/DataFormatsMath_xr.cc -cxxmodule -rmf out/DataFormatsMath_xr.rootmap -rml libDataFormatsMath.so -m DataFormatsCommon_xr_rdict.pcm -m DataFormatsProvenance_xr_rdict.pcm -m FWCoreMessageLogger_xr_rdict.pcm -m DataFormatsStdDictionaries_xr_rdict.pcm -m DataFormatsStdDictionaries_x1r_rdict.pcm -m DataFormatsStdDictionaries_x2r_rdict.pcm -m DataFormatsStdDictionaries_x3r_rdict.pcm -inlineInputHeader -failOnWarnings -I/build/yuka/genreflex/build-cmssw-tool-conf/CMSSW_10_5_CXXMODULE_X_2019-01-10-2300/src -I/cvmfs/cms-ib.cern.ch/week0/slc7_amd64_gcc700/cms/cmssw/CMSSW_10_5_CXXMODULE_X_2019-01-10-2300/src  -I/build/yuka/genreflex/build-cmssw-tool-conf/slc7_amd64_gcc700/external/boost/1.67.0/include  -DCMS_DICT_IMPL -D_REENTRANT -DGNUSOURCE -D__STRICT_ANSI__ -DGNU_GCC -D_GNU_SOURCE -DTBB_USE_GLIBCXX_VERSION=70301 -DBOOST_SPIRIT_THREADSAFE -DPHOENIX_THREADSAFE -DCMSSW_GIT_HASH=CMSSW_10_5_CXXMODULE_X_2019-01-10-2300 -DPROJECT_NAME=CMSSW -DPROJECT_VERSION=CMSSW_10_5_CXXMODULE_X_2019-01-10-2300 -DCMSSW_REFLEX_DICT build-cmssw-tool-conf/CMSSW_10_5_ROOT6_X_2019-01-14-2300/src/DataFormats/Math/src/classes.h build-cmssw-tool-conf/CMSSW_10_5_ROOT6_X_2019-01-14-2300/src/DataFormats/Math/src/classes_def.xml

```


### Small notes
gdb is at `/cvmfs/cms.cern.ch/slc7_amd64_gcc700/external/gdb/8.0/bin/gdb`


* Useful words

CMSSW - CMS SoftWare.
scram - CMSSW build system. Equivalent to cmake
IB - Integration Build. It builts every possible software stack of CMS and tests everything. Modules IB runs twice per week.

* Useful Links

https://github.com/cms-sw
https://cmssdt.cern.ch/SDT/
[List of PCMs for CMSSW](https://gist.github.com/yamaguchi1024/81133516da6c50f6ac6282041265630b)
[LD_LIBRARY_PATH of cmsenv](https://gist.github.com/yamaguchi1024/b2435ac1b4a80ea4ccf8f17e6dffd38b)

* Graphs

[Graphs](https://cmssdt.cern.ch/SDT/jenkins-artifacts/ib-dqm-tests/CMSSW_10_5_CXXMODULE_X_2019-01-08-2300/slc7_amd64_gcc700/150/memory_25202.0/mbGraph.html#?profile=performance.json&pid=_sum)

