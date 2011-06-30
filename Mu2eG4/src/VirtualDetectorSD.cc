//
// Define a sensitive detector for virtual detectors
//
// $Id: VirtualDetectorSD.cc,v 1.16 2011/06/30 04:55:13 kutschke Exp $
// $Author: kutschke $
// $Date: 2011/06/30 04:55:13 $
//
// Original author Ivan Logashenko
//

#include <cstdio>

// Framework includes
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "cetlib/exception.h"

// Mu2e includes
#include "Mu2eG4/inc/VirtualDetectorSD.hh"
#include "Mu2eG4/inc/PhysicsProcessInfo.hh"
#include "Mu2eUtilities/inc/SimpleConfig.hh"

// G4 includes
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"
#include "G4ios.hh"

using namespace std;

namespace mu2e {

  G4ThreeVector VirtualDetectorSD::_mu2eOrigin;

  VirtualDetectorSD::VirtualDetectorSD(G4String name, const SimpleConfig& config):
    G4VSensitiveDetector(name),
    _collection(0),
    _processInfo(0),
    _debugList(0),
    _sizeLimit(config.getInt("g4.stepsSizeLimit",0)),
    _currentSize(0),
    _simID(0),
    _productGetter(0)
  {

    // Get list of events for which to make debug printout.
    string key("g4.virtualSDEventList");
    if ( config.hasName(key) ){
      vector<int> list;
      config.getVectorInt(key,list);
      _debugList.add(list);
    }

  }


  VirtualDetectorSD::~VirtualDetectorSD(){ }

  void VirtualDetectorSD::Initialize(G4HCofThisEvent* HCE){

    _currentSize=0;

  }


  G4bool VirtualDetectorSD::ProcessHits(G4Step* aStep,G4TouchableHistory*){

    _currentSize += 1;

    if( _sizeLimit>0 && _currentSize>_sizeLimit ) {
      if( (_currentSize - _sizeLimit)==1 ) {
        mf::LogWarning("G4") << "Maximum number of particles reached in VirtualDetectorSD: "
                              << _currentSize << endl;
      }
      return false;
    }

    //G4Event const* event = G4RunManager::GetRunManager()->GetCurrentEvent();
    //const G4TouchableHandle & touchableHandle = aStep->GetPreStepPoint()->GetTouchableHandle();
    //G4int eventId = event->GetEventID();
    //G4int trackId = aStep->GetTrack()->GetTrackID();
    //G4int copyNo = touchableHandle->GetCopyNumber();

    // Which process caused this step to end?
    G4String const& pname  = aStep->GetPostStepPoint()->GetProcessDefinedStep()->GetProcessName();
    ProcessCode endCode(_processInfo->findAndCount(pname));

    // Add the hit to the framework collection.
    // The point's coordinates are saved in the mu2e coordinate system.
    _collection->
      push_back(StepPointMC(art::Ptr<SimParticle>( *_simID, aStep->GetTrack()->GetTrackID(), _productGetter ),
                            aStep->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetCopyNo(),
                            aStep->GetTotalEnergyDeposit(),
                            aStep->GetNonIonizingEnergyDeposit(),
                            aStep->GetPreStepPoint()->GetGlobalTime(),
                            aStep->GetPreStepPoint()->GetProperTime(),
                            aStep->GetPreStepPoint()->GetPosition() - _mu2eOrigin,
                            aStep->GetPreStepPoint()->GetMomentum(),
                            aStep->GetStepLength(),
                            endCode
                            ));

//     int static const verbosityLevel = 1;
//     if (verbosityLevel >0) {
//       cout << __func__ << " Event " << 
//         G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID() <<
//         " VD " << aStep->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetName() << " " <<
//         aStep->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetCopyNo() <<
//         " hit at: " << aStep->GetPreStepPoint()->GetPosition() - _mu2eOrigin << endl;
//     }

    return true;

  }


  void VirtualDetectorSD::EndOfEvent(G4HCofThisEvent*){

    if( _sizeLimit>0 && _currentSize>=_sizeLimit ) {
      mf::LogWarning("G4") << "Total of " << _currentSize
                            << " virtual detector hits were generated in the event."
                            << endl
                            << "Only " << _sizeLimit << " are saved in output collection."
                            << endl;
      cout << "Total of " << _currentSize
           << " virtual detector hits were generated in the event."
           << endl
           << "Only " << _sizeLimit << " are saved in output collection."
           << endl;
    }

    if (verboseLevel>0) {
      G4int NbHits = _collection->size();
      G4cout << "\n-------->Hits Collection: in this event they are " << NbHits
             << " hits in the virtual detectors: " << G4endl;
      for (G4int i=0;i<NbHits;i++) (*_collection)[i].print(G4cout);
    }

  }


  void VirtualDetectorSD::beforeG4Event(StepPointMCCollection& outputHits, 
                                        PhysicsProcessInfo& processInfo,
                                        art::ProductID const& simID,
                                        art::EDProductGetter const* productGetter ){
    _collection    = &outputHits;
    _processInfo   = &processInfo;
    _simID         = &simID;
    _productGetter = productGetter;
    return;
  } // end of beforeG4Event


} //namespace mu2e
