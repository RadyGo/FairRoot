void run_sim(Int_t nEvents = 100, TString mcEngine = "TGeant4", Bool_t isMT = false)
{

    // Output file name
    TString outFile = "test.root";

    // Parameter file name
    TString parFile = "params.root";

    // -----   Timer   --------------------------------------------------------
    TStopwatch timer;
    timer.Start();
    // ------------------------------------------------------------------------

    // -----   Create simulation run   ----------------------------------------
    FairRunSim* run = new FairRunSim();
    run->SetName(mcEngine);                        // Transport engine
    run->SetSink(new FairRootFileSink(outFile));   // Output file
    run->SetIsMT(isMT);
    FairRuntimeDb* rtdb = run->GetRuntimeDb();
    // ------------------------------------------------------------------------

    // -----   Create media   -------------------------------------------------
    run->SetMaterials("media.geo");   // Materials
    // ------------------------------------------------------------------------

    // -----   Create geometry   ----------------------------------------------

    FairModule* cave = new MyCave("CAVE");
    cave->SetGeometryFileName("cave.geo");
    run->AddModule(cave);

    FairModule* magnet = new MyMagnet("Magnet");
    run->AddModule(magnet);

    FairModule* pipe = new MyPipe("Pipe");
    run->AddModule(pipe);

    FairDetector* NewDet = new NewDetector("TestDetector", kTRUE);
    run->AddModule(NewDet);

    // ------------------------------------------------------------------------

    // -----   Magnetic field   -------------------------------------------
    // Constant Field
    MyConstField* fMagField = new MyConstField();
    fMagField->SetField(0., 20., 0.);                             // values are in kG
    fMagField->SetFieldRegion(-200, 200, -200, 200, -200, 200);   // values are in cm
                                                                  //  (xmin,xmax,ymin,ymax,zmin,zmax)
    run->SetField(fMagField);
    // --------------------------------------------------------------------

    // -----   Create PrimaryGenerator   --------------------------------------
    FairPrimaryGenerator* primGen = new FairPrimaryGenerator();

    // Add a box generator also to the run
    FairBoxGenerator* boxGen = new FairBoxGenerator(13, 5);   // 13 = muon; 1 = multipl.
    boxGen->SetPRange(20, 25);                                // GeV/c
    boxGen->SetPhiRange(0., 360.);                            // Azimuth angle range [degree]
    boxGen->SetThetaRange(0., 90.);                           // Polar angle in lab system range [degree]
    boxGen->SetXYZ(0., 0., 0.);                               // cm
    primGen->AddGenerator(boxGen);

    run->SetGenerator(primGen);
    // ------------------------------------------------------------------------

    //---Store the visualiztion info of the tracks, this make the output file very large!!
    //--- Use it only to display but not for production!
    run->SetStoreTraj(kTRUE);

    // -----   Initialize simulation run   ------------------------------------
    run->Init();
    // ------------------------------------------------------------------------

    // -----   Runtime database   ---------------------------------------------

    Bool_t kParameterMerged = kTRUE;
    FairParRootFileIo* parOut = new FairParRootFileIo(kParameterMerged);
    parOut->open(parFile.Data());
    rtdb->setOutput(parOut);
    rtdb->saveOutput();
    rtdb->print();
    // ------------------------------------------------------------------------

    // -----   Start run   ----------------------------------------------------
    run->Run(nEvents);

    // You can export your ROOT geometry ot a separate file
    run->CreateGeometryFile("geofile_full.root");
    // ------------------------------------------------------------------------

    // -----   Finish   -------------------------------------------------------
    timer.Stop();
    Double_t rtime = timer.RealTime();
    Double_t ctime = timer.CpuTime();
    cout << endl << endl;
    cout << "Macro finished succesfully." << endl;
    cout << "Output file is " << outFile << endl;
    cout << "Parameter file is " << parFile << endl;
    cout << "Real time " << rtime << " s, CPU time " << ctime << "s" << endl << endl;
    // ------------------------------------------------------------------------
}
