//
// Free function to create the calorimeter.
//
// $Id: constructCalorimeter.cc,v 1.1 2010/04/13 23:10:18 kutschke Exp $
// $Author: kutschke $
// $Date: 2010/04/13 23:10:18 $
//
// Original author Rob Kutschke
// 
// Notes
// 1) The argument zOff is the zlocation of the center of the mother volume,
//    as mesaured in the mu2e coordinate system.

#include <iostream>

// Mu2e includes.
#include "Mu2eG4/inc/constructCalorimeter.hh"
#include "Mu2eG4/inc/nestTubs.hh"
#include "Mu2eG4/inc/MaterialFinder.hh"
#include "Mu2eUtilities/inc/SimpleConfig.hh"

// G4 includes
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4Colour.hh"

using namespace std;

namespace mu2e {

  VolumeInfo constructCalorimeter( G4LogicalVolume*    mother,
                                   double              zOffset,
                                   SimpleConfig const& config ){

    // A helper class for parsing the config file.
    MaterialFinder materialFinder(config);

    // Parse the configuration file.
    double rIn               = config.getDouble("calorimeter.innerRadius");
    double rOut              = config.getDouble("calorimeter.outerRadius");
    double halfLength        = config.getDouble("calorimeter.halfLength");
    double z0                = config.getDouble("calorimeter.z0");
    G4Material* fillMaterial = materialFinder.get("calorimeter.fillMaterial");

    // Make a TUBs to represent the calorimeter mother volume.
    double calorimeterParams[5] = {
      rIn,
      rOut,
      halfLength,
      0.,
      2.*M_PI
    };

    VolumeInfo calorimeterInfo = nestTubs( "CalorimeterMother",
                                           calorimeterParams,
                                           fillMaterial,
                                           0,
                                           G4ThreeVector(0.,0.,z0+zOffset),
                                           mother,
                                           0,
                                           G4Colour::Yellow(),
                                           true
                                           );
    return calorimeterInfo;
  }

} // end namespace mu2e
