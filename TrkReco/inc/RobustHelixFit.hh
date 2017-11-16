//
// Object to perform helix fit to straw hits
//
// $Id: HelixFit.hh,v 1.8 2014/07/10 14:47:26 brownd Exp $
// $Author: brownd $ 
// $Date: 2014/07/10 14:47:26 $
//
#ifndef TrkReco_RobustHelixFit_HH
#define TrkReco_RobustHelixFit_HH


#include "fhiclcpp/ParameterSet.h"
#include "DataProducts/inc/Helicity.hh"
#include "RecoDataProducts/inc/StrawHitCollection.hh"
#include "RecoDataProducts/inc/HelixHit.hh"
#include "RecoDataProducts/inc/HelixSeed.hh"
#include "RecoDataProducts/inc/CaloClusterCollection.hh"
#include "BTrk/TrkBase/TrkErrCode.hh"
#include "TH1F.h"


namespace mu2e 
{
  // simple struct to keep track of azimuth/radius projection
  struct FZ 
  {
    double _phi;
    double _z;
    FZ(CLHEP::Hep3Vector const& hpos, CLHEP::Hep3Vector const& center);
  };

  // struct to hold AGE sums
  struct AGESums {
    // (s)ums of (c)osine and (s)in for points on (c)ircumference, (o)utside the median radius, or (i)nside the median radius
    double _scc, _ssc, _sco, _sso, _sci, _ssi;
    unsigned _nc, _no, _ni;
    AGESums() : _scc(0.0),_ssc(0.0),_sco(0.0),_sso(0.0),_sci(0.0),_ssi(0.0){}
    void clear() { _scc = _ssc = _sco = _sso = _sci = _ssi = 0.0;
      _nc = _no = _ni = 0; }
  };


  class RobustHelixFit
  {
    public:
      
      enum CircleFit {median=0, AGE=1, chisq=2 };

      explicit RobustHelixFit(fhicl::ParameterSet const&);
      virtual ~RobustHelixFit();

      void fitCircle(HelixSeed& hseed, const CaloClusterCollection* caloClusters);
      void fitFZ(HelixSeed& hseed);
      bool goodHelix(RobustHelix const& rhel);
      Helicity const& helicity() const { return _helicity; }

    private:

      void fitHelix(HelixSeed& hseed, const CaloClusterCollection* caloClusters);
      void fitCircleMedian(HelixSeed& hseed, const CaloClusterCollection* caloClusters);
      void fitCircleAGE(HelixSeed& hseed, const CaloClusterCollection* caloClusters);
      bool initFZ(HelixHitCollection& hhits,RobustHelix& myhel);
      void findAGE(HelixSeed const& hseed, CLHEP::Hep3Vector const& center,double& rmed, double& age);
      void fillSums(HelixSeed const& hseed, CLHEP::Hep3Vector const& center,double rmed,AGESums& sums);

      bool goodCircle(RobustHelix const& rhel);
      bool goodFZ(RobustHelix const& rhel);

      void forceTargetInter(CLHEP::Hep3Vector& center, double& radius);

      bool use(HelixHit const&) const;
      bool stereo(HelixHit const&) const;
      void setOutlier(HelixHit&) const;

      static double deltaPhi(double phi1, double phi2);
      void initPhi(HelixHit& hh, RobustHelix const& myhel) const;
      bool resolvePhi(HelixHit& hh, RobustHelix const& myhel) const;
      double hitWeight(HelixHit const& hhit) const;

      int _debug;
      CircleFit _cfit; // type of circle fit
      StrawHitFlag _useflag, _dontuseflag;
      unsigned _minnhit; // minimum # of hits to work with
      double _lambda0,_lstep,_minlambda; // parameters for AGE center determination
      unsigned _nphibins; // # of bins in histogram for phi at z intercept
      double _phifactor; // range factr for phi z intercept histogram 
      unsigned _minnphi; // minimum # of entries in max bin of phi intercept histogram 
      unsigned _maxniter; // maxium # of iterations to global minimum
      double _minzsep, _maxzsep; // Z separation of points for pitch estimate
      double _mindphi, _maxdphi; // phi separation of points for pitch estimate
      double _mindist; // minimum distance between points used in circle initialization
      double _maxdist; // maximum distance in hits
      double _rmin,_rmax; // circle radius range
      double _mindelta; // minimum slope difference to use a triple in circle center initialization
      double _lmin, _lmax; // range of lambda = dz/dphi
      bool _stereoinit; // require stereo hits to initialize
      bool _stereofit; // require stereo hits 
      bool _targetpoint; // use target as a point in the circle fit
      bool _targetinit; // require consistency with target when initializing circle
      bool _targetinter; // require fit to intersect the target
      bool _usecc; // use the calorimeter cluster in the fit (transverse only)
      double _ccwt; // weight of a calorimeter cluster in non-stereo hit units
      double _stwt; // weight of a stereo hit (wrt non-stereo)
      bool _hqwt; // weight hits by 'quality'
      double _targetradius; // target size to use in constraint or init
      double _trackerradius; // tracker radius to use in init
      double _rwind; // raidus window for defining points to be 'on' the helix
      Helicity _helicity; // helicity value to look for.  This defines the sign of dphi/dz
      TH1F _hphi;
      unsigned _ntripleMax;
 };
}
#endif
