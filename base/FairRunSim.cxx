// -------------------------------------------------------------------------
// -----                   FairRunSim source file                      -----
// -----            Created 06/01/04  by M. Al-Turany                  -----
// -------------------------------------------------------------------------


#include "FairRunSim.h"
#include "FairRootManager.h"
#include "FairTrajFilter.h"
#include "FairBaseParSet.h"
#include "FairRunIdGenerator.h"
#include "FairModule.h"
#include "FairField.h"
#include "FairRuntimeDb.h"
#include "FairGeoLoader.h"
#include "FairGeoInterface.h"
#include "FairMCEventHeader.h"
#include "FairPrimaryGenerator.h"
#include "TROOT.h"
#include "TSystem.h"
#include <iostream>
using std::cout;
using std::endl;

ClassImp(FairRunSim)

FairRunSim::FairRunSim()
  :FairRun(),
   count(0),
   fApp(NULL),
   fBeamMom(0),
   fUseBeamMom(kFALSE),
   fGen(NULL),
   fMCEvHead(NULL),
   fField(NULL),
   fMapName(""),
   fIons(new TObjArray()),
   fParticles( new TObjArray()),
   ListOfModules(new TObjArray()),
   MatFname(""),
   fStoreTraj( kFALSE),
   fLoaderName( new TString("TGeo")),
   fPythiaDecayer(kFALSE),
   fUserDecay(kFALSE),
   fRadLength(kFALSE),
   fUserConfig(""),
   fUserCuts("SetCuts.C")

{
  if (fginstance) {
    Fatal("FairRun", "Singleton instance already exists.");
    return;
  }
  Outfname="";
  fginstance=this;
  fRunId=0;
  fAna=kFALSE;
}

FairRunSim::~FairRunSim()
{
  fLogger->Debug(MESSAGE_ORIGIN," Enter Destructor of FairRunSim ");
  // List of Modules is filled via AddModule from the macro, but it
  // is the responsibility of FairRunSim to call the destructors of
  // the modules-
  fLogger->Debug(MESSAGE_ORIGIN," Start deleting all registered modules");
  ListOfModules->Delete();
  delete ListOfModules;
  fLogger->Debug(MESSAGE_ORIGIN," Finish deleting all registered modules");

  fIons->Delete();
  delete fIons;
  fParticles->Delete();
  delete fParticles;

  delete fApp;
  delete fField;
  delete fGen;
  delete fMCEvHead;
}
FairRunSim* FairRunSim::Instance()
{
  return fginstance;
}
void FairRunSim::AddModule (FairModule* Mod)
{
  ListOfModules->Add(Mod);
  Mod->SetModId(count++);
}

TObjArray* FairRunSim::GetUserDefIons()
{
  /** return the array of user defined ions*/
  return fIons;
}

TObjArray* FairRunSim::GetUserDefParticles()
{
  /** return the array of user defined particles*/
  return fParticles;
}

void FairRunSim::Init()
{
  /**Initialize the simulation session*/

  CheckFlukaExec();

  fOutFile=fRootManager->OpenOutFile(Outfname);
  fLogger->Info(MESSAGE_ORIGIN,"==============  FairRunSim: Initialising simulation run ==============");

  FairGeoLoader* loader=new FairGeoLoader(fLoaderName->Data(), "Geo Loader");
  FairGeoInterface* GeoInterFace=loader->getGeoInterface();
  GeoInterFace->SetNoOfSets(ListOfModules->GetEntries());
  GeoInterFace->setMediaFile(MatFname.Data());
  GeoInterFace->readMedia();

  //  gSystem->cd(flout.Data());

  fApp= new FairMCApplication("Fair","The Fair VMC App",ListOfModules, MatFname);
  fApp->SetGenerator(fGen);

  // Add a Generated run ID to the FairRunTimeDb
  FairRunIdGenerator genid;
  // FairRuntimeDb *rtdb= GetRuntimeDb();
  fRunId = genid.generateId();
  fRtdb->addRun(fRunId);


  /** Add Tasks to simulation if any*/
  fApp->AddTask(fTask);


  FairBaseParSet* par=(FairBaseParSet*)(fRtdb->getContainer("FairBaseParSet"));
  par->SetDetList(GetListOfModules());
  par->SetGen(GetPrimaryGenerator());
  par->SetBeamMom(fBeamMom);
  par->SetGeometry(gGeoManager);



  // Set global Parameter Info

  if(fPythiaDecayer) {
    fApp->SetPythiaDecayer(fPythiaDecayer);
  }
  if(fUserDecay) {
    fApp->SetUserDecay(fUserDecay);
  }
  // on/off visualisation
  if( fStoreTraj ) {
    fLogger->Info(MESSAGE_ORIGIN, "Create visualisation manager ");
    new FairTrajFilter();
  }
  if(fRadLength) {
    fApp->SetRadiationLengthReg(fRadLength);
  }

  if(fField) { fField->Init(); }
  fApp->SetField(fField);
  SetFieldContainer();



  TList* containerList=fRtdb->getListOfContainers();
  TIter next(containerList);
  FairParSet* cont;
  TObjArray* ContList= new TObjArray();
  while ((cont=(FairParSet*)next())) {
    ContList->Add(new TObjString(cont->GetName()));
  }

  par->SetContListStr(ContList);
  par->setChanged();
  par->setInputVersion(fRunId,1);


  /**Set the configuration for MC engine*/
  SetMCConfig();


}

void FairRunSim::SetFieldContainer()
{

  //  FairRuntimeDb *rtdb=fRun->GetRuntimeDb();
//  Bool_t kParameterMerged=kTRUE;

//  PndMultiFieldPar* Par = (PndMultiFieldPar*) rtdb->getContainer("PndMultiFieldPar");
//  if (fField) {  Par->SetParameters(fField); }
//  Par->setInputVersion(fRun->GetRunId(),1);
//  Par->setChanged();


  //printf("###################FairRunSim::SetFieldContainer() ######################\n");
  if (fField) {
    fField->FillParContainer();

  }


}



void FairRunSim::CheckFlukaExec()
{
  /** Private method for setting FLUKA simulation*/

  TString work = getenv("VMCWORKDIR");
  TString work_config=work+"/gconfig/";
  TString config_dir= getenv("CONFIG_DIR");
  if (!config_dir.EndsWith("/")) { config_dir+="/"; }

  TString flout;
  if(strcmp(GetName(),"TFluka") == 0 ) {
    TString flexec="run_fluka.sh";
    if (TString(gSystem->FindFile(config_dir.Data(),flexec)) != TString("")) {
      fLogger->Info(MESSAGE_ORIGIN,"---User path for Configuration is used :  %s ", config_dir.Data());
    } else {
      flexec=work_config+"run_fluka.sh";
    }
    gSystem->Exec(flexec.Data());
    // More generic for different macro
    // dir layout <D.B>
    TString macrodir = getenv("PWD");
    cout << macrodir << endl;
    flout = macrodir + "/fluka_out" ;
    gSystem->cd(flout.Data());
  }


}

void FairRunSim::SetMCConfig()
{
  /** Private method for setting simulation and/or Geane configuration and cuts*/

  TString work = getenv("VMCWORKDIR");
  TString Lib_config= getenv("GEANT4VMC_MACRO_DIR");
  TString work_config=work+"/gconfig/";
  TString config_dir= getenv("CONFIG_DIR");
  Bool_t AbsPath=kFALSE;
  if (!config_dir.EndsWith("/")) { config_dir+="/"; }
  if (!Lib_config.EndsWith("/")) { Lib_config+="/"; }

  TString LibMacro;
  TString LibFunction;
  TString ConfigMacro;
  TString cuts=fUserCuts;
  //----------------------------------------------Geant4 Config-----------------------------------------
  if(strcmp(GetName(),"TGeant4") == 0 ) {
    TString g4LibMacro="g4libs.C";
    TString g4Macro;
    if(fUserConfig.IsNull()) {
      g4Macro="g4Config.C";
      fUserConfig = g4Macro;
    } else {
      if (fUserConfig.Contains("/")) { AbsPath=kTRUE; }
      g4Macro = fUserConfig;
      fLogger->Info(MESSAGE_ORIGIN,"---------------User config is used : %s ",g4Macro.Data());
    }
    if (TString(gSystem->FindFile(config_dir.Data(),g4LibMacro)) != TString("")) { //be carfull after this call the string g4LibMacro is empty if not found!!!!
      fLogger->Info(MESSAGE_ORIGIN,"---User path for Configuration (g4libs.C) is used : %s" ,config_dir.Data());
    } else {
      g4LibMacro=Lib_config+"g4libs.C";
    }
    LibMacro=g4LibMacro;
    LibFunction="g4libs()";
    if (!AbsPath && TString(gSystem->FindFile(config_dir.Data(),g4Macro)) != TString("")) {
      fLogger->Info(MESSAGE_ORIGIN,"---User path for Configuration (g4Config.C) is used : %s" , config_dir.Data());
      ConfigMacro=g4Macro;
    } else {
      if(AbsPath) { ConfigMacro = fUserConfig; }
      else { ConfigMacro =work_config+fUserConfig; }
    }
    //----------------------------------------------Geant3 Config-----------------------------------------
  } else if(strcmp(GetName(),"TGeant3") == 0 ) {
    TString g3LibMacro="g3libs.C";
    TString g3Macro="g3Config.C";
    if(fUserConfig.IsNull()) {
      g3Macro="g3Config.C";
      fUserConfig = g3Macro;
      fLogger->Info(MESSAGE_ORIGIN,"-------------- Standard Config is called ------------------------------------");
    } else {
      if (fUserConfig.Contains("/")) { AbsPath=kTRUE; }
      g3Macro = fUserConfig;
      fLogger->Info(MESSAGE_ORIGIN,"---------------User config is used :  ", g3Macro.Data());
    }
    if (TString(gSystem->FindFile(config_dir.Data(),g3LibMacro)) != TString("")) {
      fLogger->Info(MESSAGE_ORIGIN,"---User path for Configuration (g3libs.C) is used : %s " , config_dir.Data());
    } else {
      g3LibMacro=work_config+"g3libs.C";
    }
    LibMacro=g3LibMacro;
    LibFunction="g3libs()";
    if (!AbsPath && TString(gSystem->FindFile(config_dir.Data(),g3Macro)) != TString("")) {
      fLogger->Info(MESSAGE_ORIGIN,"---User path for Configuration (g3Config.C) is used : %s", config_dir.Data());
      ConfigMacro=g3Macro;
    } else {
      if(AbsPath) { ConfigMacro = fUserConfig; }
      else { ConfigMacro =work_config+fUserConfig; }
    }
    //----------------------------------------------Fluka Config-----------------------------------------
  } else if(strcmp(GetName(),"TFluka") == 0 ) {
    TString flLibMacro="fllibs.C";
    TString flMacro="flConfig.C";
    if(fUserConfig.IsNull()) {
      flMacro="flConfig.C";
      fUserConfig=flMacro;
    } else {
      if (fUserConfig.Contains("/")) { AbsPath=kTRUE; }
      flMacro = fUserConfig;
      fLogger->Info(MESSAGE_ORIGIN,"---------------User config is used : %s " , flMacro.Data());
    }
    if (TString(gSystem->FindFile(config_dir.Data(), flLibMacro)) != TString("")) {
      fLogger->Info(MESSAGE_ORIGIN,"---User path for Configuration (fllibs.C) is used : %s", config_dir.Data());
    } else {
      flLibMacro=work_config+"fllibs.C";
    }
    LibMacro=flLibMacro;
    LibFunction="fllibs()";
    if (!AbsPath && TString(gSystem->FindFile(config_dir.Data(),flMacro)) != TString("")) {
      fLogger->Info(MESSAGE_ORIGIN, "---User path for Configuration (flConfig.C) is used : %s",  config_dir.Data());
      ConfigMacro=flMacro;
    } else {
      if(AbsPath) { ConfigMacro = fUserConfig; }
      else { ConfigMacro =work_config+fUserConfig; }
    }
  }
  //----------------------------------------------SetCuts------------------------------------------------
  if (TString(gSystem->FindFile(config_dir.Data(),cuts)) != TString("")) {
    fLogger->Info(MESSAGE_ORIGIN,"---User path for Cuts and Processes  (SetCuts.C) is used : %s", config_dir.Data());
  } else {
    cuts =work_config+ fUserCuts;
  }
  //--------------------------------------Now load the Config and Cuts------------------------------------
  gROOT->LoadMacro(LibMacro.Data());
  gROOT->ProcessLine(LibFunction.Data());

  gROOT->LoadMacro(ConfigMacro.Data());
  gROOT->ProcessLine("Config()");

  gROOT->LoadMacro(cuts);
  gROOT->ProcessLine("SetCuts()");

  fApp->InitMC(ConfigMacro.Data(), cuts.Data());
}


void FairRunSim::Run(Int_t NEvents, Int_t NotUsed)
{


  fApp->RunMC(NEvents);
}
void FairRunSim::SetField(FairField* field)
{
  fField=field;
}
void FairRunSim::SetGenerator(FairPrimaryGenerator* Gen)
{
  fGen=Gen;
}

void FairRunSim::SetMaterials(const char* MatFileName)
{
  TString Mat="";
  TString work = getenv("GEOMPATH");
  if (work.IsNull()) {
    work = getenv("VMCWORKDIR");
    Mat=work+"/geometry/";
  } else {
    Mat=work;
    if (!Mat.EndsWith("/")) { Mat+="/"; }
  }
  MatFname=Mat+MatFileName;
  fLogger->Info(MESSAGE_ORIGIN,"Media file used : %s ", MatFname.Data());
}

void FairRunSim::SetGeoModel( char* name )
{
  if ( strncmp(fName,"TGeant3",7) == 0 ) {
    if (fLoaderName) { delete fLoaderName; }
    fLoaderName = new TString(name);
    fLogger->Info(MESSAGE_ORIGIN,"FairRun::SetGeoModel(): G3 native geometry model used ");
  } else {
    fLogger->Info(MESSAGE_ORIGIN,"FairRun::SetGeoModel(): Geant3 MC engine only !");
    fLogger->Info(MESSAGE_ORIGIN,"FairRun::SetGeoModel(): Method skipped ... ");
  }
}


void  FairRunSim::SetPythiaDecayer(const TString& Config )
{
  /**switch On external decayer (Pythia). Config macro will be used */
  fPythiaDecayerConfig = Config;
  fPythiaDecayer =kTRUE;
}


void  FairRunSim::SetUserDecay(const TString& Config)
{
  /**switch On user defined decay, Config  macro will be called  */
  fUserDecayConfig = Config;
  fUserDecay = kTRUE;
}

FairMCEventHeader*  FairRunSim::GetMCEventHeader()
{
  if ( NULL == fMCEvHead ) { fMCEvHead = new FairMCEventHeader(); }
  return fMCEvHead;
}


FairRunSim* FairRunSim::fginstance= 0;



