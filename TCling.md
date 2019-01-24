### TCling::TCling()

Many things are happening in TCling constructor. First, we add flags to pass to cling and initialize fInterpreter pointer, and we load modules (LoadModules) afterwards.

```
   // rootcling sets its arguments through TROOT::GetExtraInterpreterArgs().
   if (!fromRootCling) {
```
rootcling is a dictionary generator, different binary from ROOT. It is benefitial just to remember that TCling constructor can be called by both ROOT and rootcling.

```
   // Attach the PCH (unless we have C++ modules enabled which provide the
   // same functionality).
   if (!fCxxModulesEnabled) {
      std::string pchFilename = interpInclude + "/allDict.cxx.pch";
      if (gSystem->Getenv("ROOT_PCH")) {
         pchFilename = gSystem->Getenv("ROOT_PCH");
      }

      clingArgsStorage.push_back("-include-pch");
      clingArgsStorage.push_back(pchFilename);
   }
```
This part of code is used to load PCH. pch is basically a dump of AST, which is created at the end of compilation time.

```
   if (fCxxModulesEnabled) {
      clingArgsStorage.push_back("-modulemap_overlay=" + std::string(TROOT::GetIncludeDir().Data()));
   }
```
We have a virtual modulemap overlay file, which is created and loaded in Interpreter::Interpreter(). clingArgsStorage is a vector which stores flags to Cling, so we pass `-modulemap_overlay=` flag with the location of overlay file (eg. stl.modulemap, libc.modulemap)

```
   fInterpreter = new cling::Interpreter(interpArgs.size(),
                                         &(interpArgs[0]),
                                         llvmResourceDir);
```
This is where interpreter initialization is happening. Please look into Interpreter.md for more details. Many things are happening here.

```
   if (!fromRootCling) {
      fInterpreter->installLazyFunctionCreator(llvmLazyFunctionCreator);
   }
```
This is where we're registering llvmLazyFunctionCreator which desends to LazyFunctionCreatorAutoloadForModule. Please look into LazyFunctionCreatorAutoloadForModule section for detailed information. In short, LazyFunctionCreatorAutoloadForModule gets a callback when a mangled name couldn't be resolved, and is responsible for loading an appropriate library to resolve the definition.

```
   if (fInterpreter->getCI()->getLangOpts().Modules) {
      // Setup core C++ modules if we have any to setup.

      // Load libc and stl first.
      LoadModules({"libc", "stl"}, *fInterpreter);
```
This is where we're preloading modules. libc and stl are implicit modules, which needs to be loaded first as other ROOT pcms are relying on them.

```
   // Take this branch only from ROOT because we don't need to preload modules in rootcling
   if (!fromRootCling) {
      // Dynamically get all the modules and load them if they are not in core modules
      clang::CompilerInstance &CI = *fInterpreter->getCI();
      clang::ModuleMap &moduleMap = CI.getPreprocessor().getHeaderSearchInfo().getModuleMap();
      clang::Preprocessor &PP = CI.getPreprocessor();
      std::vector<std::string> ModulesPreloaded;
      for (auto I = moduleMap.module_begin(), E = moduleMap.module_end(); I != E; ++I) {
         clang::Module *M = I->second;
         assert(M);
         std::string ModuleName = GetModuleNameAsString(M, PP);
         if (!ModuleName.empty() &&
               std::find(CoreModules.begin(), CoreModules.end(), ModuleName) == CoreModules.end()
               && std::find(ExcludeModules.begin(), ExcludeModules.end(), ModuleName) == ExcludeModules.end()) {
            if (M->IsSystem && !M->IsMissingRequirement)
               LoadModule(ModuleName, *fInterpreter);
            else if (!M->IsSystem && !M->IsMissingRequirement)
               ModulesPreloaded.push_back(ModuleName);
         }
      }
      LoadModules(ModulesPreloaded, *fInterpreter);
   }
```
Preloading of all modules. We get modulemap pointer, iterate through and pass everything to LoadModules.

### LoadModule

This is calling Clang API to load modules.
```
   bool success = !CI.getSema().ActOnModuleImport(ValidLoc, ValidLoc, std::make_pair(II, ValidLoc)).isInvalid();
```
This is where actual import is happening.

```
   // Load modulemap if we have one in current directory
   SourceManager& SM = PP.getSourceManager();
   FileManager& FM = SM.getFileManager();
   const clang::DirectoryEntry *DE = FM.getDirectory(".");
   if (DE) {
      const clang::FileEntry *FE = headerSearch.lookupModuleMapFile(DE, /*IsFramework*/ false);
      // Check if "./module.modulemap is already loaded or not
      if (!gCling->IsLoaded("./module.modulemap") && FE) {
         if(!headerSearch.loadModuleMapFile(FE, /*IsSystem*/ false))
            return LoadModule(ModuleName, interp, Complain);
         Error("TCling::LoadModule", "Could not load modulemap in the current directory");
      }
   }
```
We added this in order to fix some tests. It loads modulemap in current directory, so that we can avoid error like "This module is not defined in modulemap"

### TCling::LoadPCM

Confusing.. But PCM here is "rdict pcm", not "C++ Modules PCM" :) :) This function name is misleading. This is called by TCling::RegisterModule.

### 
