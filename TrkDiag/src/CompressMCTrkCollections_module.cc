////////////////////////////////////////////////////////////////////////
// Class:       CompressMCTrkCollections
// Plugin Type: producer (art v2_06_02)
// File:        CompressMCTrkCollections_module.cc
//
// Creates a new MC collections that have been reduced
// in size based on a given StrawHitFlag bit
//
// If you want to filter on another StrawHitFlag bit (e.g. you want to store 
// hits near to track hits), then create a new module that will create a
// new StrawHitFlagCollection and the new StrawHitFlag bit. Then you can pass
// the new StrawHitFlagCollection and StrawHitFlag bit to this module.
//
//
// Generated at Wed Apr 12 16:10:46 2017 by Andrew Edmonds using cetskelgen
// from cetlib version v2_02_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <memory>

#include "RecoDataProducts/inc/ComboHit.hh"

#include "MCDataProducts/inc/StrawDigiMCCollection.hh"
#include "MCDataProducts/inc/StepPointMCCollection.hh"
#include "MCDataProducts/inc/SimParticleCollection.hh"
#include "Mu2eUtilities/inc/compressSimParticleCollection.hh"
#include "MCDataProducts/inc/GenParticleCollection.hh"
#include "MCDataProducts/inc/SimParticleTimeMap.hh"

namespace mu2e {
  class CompressMCTrkCollections;

  class SimParticleSelector {
  public:
    SimParticleSelector() { }
    
    void push_back(cet::map_vector_key key) {
      m_keys.insert(key);
    }

    bool operator[]( cet::map_vector_key key ) const {
      return m_keys.find(key) != m_keys.end();
    }

    std::set<cet::map_vector_key>& keys() {
      return m_keys;
    }

    void clear() {
      m_keys.clear();
    }

  private:
    std::set<cet::map_vector_key> m_keys;
    
  };
}


class mu2e::CompressMCTrkCollections : public art::EDProducer {
public:
  explicit CompressMCTrkCollections(fhicl::ParameterSet const & pset);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  CompressMCTrkCollections(CompressMCTrkCollections const &) = delete;
  CompressMCTrkCollections(CompressMCTrkCollections &&) = delete;
  CompressMCTrkCollections & operator = (CompressMCTrkCollections const &) = delete;
  CompressMCTrkCollections & operator = (CompressMCTrkCollections &&) = delete;

  // Required functions.
  void produce(art::Event & event) override;

  // Other functions
  void addStrawHitMCProducts(StrawHitIndex index);
  void copyStrawDigiMC(const mu2e::StrawDigiMC& old_straw_digi_mc);
  art::Ptr<StepPointMC> copyStepPointMC(const mu2e::StepPointMC& old_step);

private:

  // the straw hit flag that the new straw hit products need to have (see RecoDataProducts/src/StrawHitFlag.cc for the options)
  std::string _wantedHitFlag;

  // art tags for the input collections
  art::InputTag _comboHitTag;
  art::InputTag _strawDigiMCTag;
  std::vector<art::InputTag> _simParticleTags;
  art::InputTag _extraStepPointMCTag;
  std::vector<int> _extraStepPointMCVolIDs;
  std::vector<art::InputTag> _timeMapTags;

  // handles to the old collections
  art::Handle<ComboHitCollection> _comboHitsHandle;
  art::Handle<StrawDigiMCCollection> _strawDigiMCsHandle;
  std::vector<SimParticleTimeMap> _oldTimeMaps;

  // unique_ptrs to the new output collections
  std::unique_ptr<StrawDigiMCCollection> _newStrawDigiMCs;
  std::unique_ptr<StepPointMCCollection> _newStepPointMCs;
  std::map<art::ProductID, std::unique_ptr<SimParticleCollection> > _newSimParticles;
  std::map<art::ProductID, std::unique_ptr<GenParticleCollection> > _newGenParticles;
  std::vector<std::unique_ptr<SimParticleTimeMap> > _newSimParticleTimeMaps;

  // keep track of the hits we are filtering
  std::map<StrawHitIndex, StrawHitIndex> _oldToNewStrawHitIndexMap;
  int _hitCounter;

  // for StepPointMCs, SimParticles and GenParticles we also need reference their new locations with art::Ptrs and so need their ProductIDs and Getters
  art::ProductID _newStepPointMCsPID;
  const art::EDProductGetter* _newStepPointMCGetter;
  std::map<art::ProductID, art::ProductID> _newSimParticlesPID;
  std::map<art::ProductID, const art::EDProductGetter*> _oldSimParticleGetter;
  std::map<art::ProductID, const art::EDProductGetter*> _newSimParticleGetter;
  std::map<art::ProductID, art::ProductID> _newGenParticlesPID;
  std::map<art::ProductID, const art::EDProductGetter*> _newGenParticleGetter;

  // record the SimParticles that we are keeping so we can use compressSimParticleCollection to do all the work for us
  std::map<art::ProductID, SimParticleSelector> _simParticlesToKeep;
};


mu2e::CompressMCTrkCollections::CompressMCTrkCollections(fhicl::ParameterSet const & pset)
  : _wantedHitFlag(pset.get<std::string>("wantedHitFlag")),
    _comboHitTag(pset.get<art::InputTag>("comboHitTag")),
    _strawDigiMCTag(pset.get<art::InputTag>("strawDigiMCTag")),
    _simParticleTags(pset.get<std::vector<art::InputTag> >("simParticleTags")),
    _extraStepPointMCTag(pset.get<art::InputTag>("extraStepPointMCTag", "")),
    _extraStepPointMCVolIDs(pset.get<std::vector<int> >("extraStepPointMCVolIDs")),
    _timeMapTags(pset.get<std::vector<art::InputTag> >("timeMapTags"))
{
  // Call appropriate produces<>() functions here.
  produces<StrawDigiMCCollection>();

  produces<StepPointMCCollection>();

  for (std::vector<art::InputTag>::const_iterator i_tag = _simParticleTags.begin(); i_tag != _simParticleTags.end(); ++i_tag) {
    produces<SimParticleCollection>( (*i_tag).label() );
    produces<GenParticleCollection>( (*i_tag).label() );
  }

  for (std::vector<art::InputTag>::const_iterator i_tag = _timeMapTags.begin(); i_tag != _timeMapTags.end(); ++i_tag) {
    produces<SimParticleTimeMap>( (*i_tag).label() );
  }
}

void mu2e::CompressMCTrkCollections::produce(art::Event & event)
{
  // Implementation of required member function here.
  _newStrawDigiMCs = std::unique_ptr<StrawDigiMCCollection>(new StrawDigiMCCollection);  
  _newStepPointMCs = std::unique_ptr<StepPointMCCollection>(new StepPointMCCollection);
  _newStepPointMCsPID = getProductID<StepPointMCCollection>();
  _newStepPointMCGetter = event.productGetter(_newStepPointMCsPID);

  _hitCounter = 0;
  _oldToNewStrawHitIndexMap.clear();

  event.getByLabel(_strawDigiMCTag, _strawDigiMCsHandle);

  // Create all the new collections, ProductIDs and product getters for the SimParticles and GenParticles
  // There is one for each background frame plus one for the primary event
  for (std::vector<art::InputTag>::const_iterator i_tag = _simParticleTags.begin(); i_tag != _simParticleTags.end(); ++i_tag) {
    const auto& oldSimParticles = event.getValidHandle<SimParticleCollection>(*i_tag);
    art::ProductID i_product_id = oldSimParticles.id();
    
    _simParticlesToKeep[i_product_id].clear();
    
    _newSimParticles[i_product_id] = std::unique_ptr<SimParticleCollection>(new SimParticleCollection);
    _newSimParticlesPID[i_product_id] = getProductID<SimParticleCollection>((*i_tag).label() );
    _newSimParticleGetter[i_product_id] = event.productGetter(_newSimParticlesPID[i_product_id]);

    _oldSimParticleGetter[i_product_id] = event.productGetter(i_product_id);
    
    _newGenParticles[i_product_id] = std::unique_ptr<GenParticleCollection>(new GenParticleCollection);
    _newGenParticlesPID[i_product_id] = getProductID<GenParticleCollection>((*i_tag).label() );
    _newGenParticleGetter[i_product_id] = event.productGetter(_newGenParticlesPID[i_product_id]);
  }

  _oldTimeMaps.clear();
  for (std::vector<art::InputTag>::const_iterator i_tag = _timeMapTags.begin(); i_tag != _timeMapTags.end(); ++i_tag) {
    art::Handle<SimParticleTimeMap> i_timeMapHandle;
    event.getByLabel(*i_tag, i_timeMapHandle);

    if (!i_timeMapHandle.isValid()) {
      throw cet::exception("CompressMCTrkCollections") << "Couldn't find SimParticleTimeMap " << *i_tag << " in event\n";
    }
    _oldTimeMaps.push_back(*i_timeMapHandle);
  }

  _newSimParticleTimeMaps.clear();
  for (std::vector<art::InputTag>::const_iterator i_tag = _timeMapTags.begin(); i_tag != _timeMapTags.end(); ++i_tag) {
    _newSimParticleTimeMaps.push_back(std::unique_ptr<SimParticleTimeMap>(new SimParticleTimeMap));
  }

  

  // Loop through the combo hits
  event.getByLabel(_comboHitTag, _comboHitsHandle);
  const auto& comboHits = *_comboHitsHandle;
  for (unsigned int i_straw_hit = 0; i_straw_hit < comboHits.size(); ++i_straw_hit) {
    const mu2e::StrawHitFlag& strawHitFlag = comboHits.at(i_straw_hit).flag();
    StrawHitIndex hit_index = i_straw_hit;
    
    // write out the StrawHits that have the StrawHitFlags we want
    if (_wantedHitFlag == "") {
      addStrawHitMCProducts(hit_index);
    }
    else {
      mu2e::StrawHitFlag wanted(_wantedHitFlag);
      if (strawHitFlag.hasAllProperties(wanted)) {
	addStrawHitMCProducts(hit_index);
      }
    }
  }
  
  // Get the hits from the virtualdetector
  if (_extraStepPointMCTag != "") {
    const auto& stepPointMCs = event.getValidHandle<StepPointMCCollection>(_extraStepPointMCTag);
    for (const auto& volID : _extraStepPointMCVolIDs) {
      for (const auto& stepPointMC : *stepPointMCs) {
	if (stepPointMC.volumeId() == static_cast<mu2e::VirtualDetectorId>(volID)) {
	  copyStepPointMC(stepPointMC);
	}
      }
    }
  }
  
  // Now compress the SimParticleCollections into their new collections
  for (std::vector<art::InputTag>::const_iterator i_tag = _simParticleTags.begin(); i_tag != _simParticleTags.end(); ++i_tag) {
    const auto& oldSimParticles = event.getValidHandle<SimParticleCollection>(*i_tag);
    art::ProductID i_product_id = oldSimParticles.id();
    compressSimParticleCollection(_newSimParticlesPID[i_product_id], _newSimParticleGetter[i_product_id], *oldSimParticles, 
				  _simParticlesToKeep[i_product_id], *(_newSimParticles[i_product_id]));

    for(auto& i : *(_newSimParticles[i_product_id])) {
      
      mu2e::SimParticle& newsim = i.second;
      if(!newsim.genParticle().isNull()) { // will crash if not resolvable

	// Copy GenParticle to the new collection
	_newGenParticles[i_product_id]->emplace_back(*newsim.genParticle());
	newsim.genParticle() = art::Ptr<GenParticle>(_newGenParticlesPID[i_product_id], _newGenParticles[i_product_id]->size()-1, _newGenParticleGetter[i_product_id]);
      }
      
      // Update the time maps
      art::Ptr<SimParticle> oldSimPtr(i_product_id, newsim.id().asUint(), _oldSimParticleGetter[i_product_id]);
      art::Ptr<SimParticle> newSimPtr(_newSimParticlesPID[i_product_id], newsim.id().asUint(), _newSimParticleGetter[i_product_id]);
      for (std::vector<SimParticleTimeMap>::const_iterator i_time_map = _oldTimeMaps.begin(); i_time_map != _oldTimeMaps.end(); ++i_time_map) {
	size_t i_element = i_time_map - _oldTimeMaps.begin();
	
	const SimParticleTimeMap& i_oldTimeMap = *i_time_map;
	SimParticleTimeMap& i_newTimeMap = *_newSimParticleTimeMaps.at(i_element);
	auto it = i_oldTimeMap.find(oldSimPtr);
	if (it != i_oldTimeMap.end()) {
	  i_newTimeMap[newSimPtr] = it->second;
	}
      }
    }
  }

  // Now add everything to the event
  event.put(std::move(_newStepPointMCs));  
  event.put(std::move(_newStrawDigiMCs));

  for (std::vector<art::InputTag>::const_iterator i_tag = _simParticleTags.begin(); i_tag != _simParticleTags.end(); ++i_tag) {
    const auto& oldSimParticles = event.getValidHandle<SimParticleCollection>(*i_tag);
    art::ProductID i_product_id = oldSimParticles.id();
    event.put(std::move(_newSimParticles[i_product_id]), (*i_tag).label());
    event.put(std::move(_newGenParticles[i_product_id]), (*i_tag).label());
  }

  for (std::vector<art::InputTag>::const_iterator i_tag = _timeMapTags.begin(); i_tag != _timeMapTags.end(); ++i_tag) {
    size_t i_element = i_tag - _timeMapTags.begin();
    event.put(std::move(_newSimParticleTimeMaps.at(i_element)), (*i_tag).label());
  }	
}

void mu2e::CompressMCTrkCollections::addStrawHitMCProducts(StrawHitIndex hit_index) {

  // Need a deep copy of StrawDigiMC
  const mu2e::StrawDigiMC& old_straw_digi_mc = _strawDigiMCsHandle->at(hit_index);
  copyStrawDigiMC(old_straw_digi_mc);

  // Update the map
  _oldToNewStrawHitIndexMap[hit_index] = _hitCounter;
  ++_hitCounter;
}

void mu2e::CompressMCTrkCollections::copyStrawDigiMC(const mu2e::StrawDigiMC& old_straw_digi_mc) {

  // Need to update the Ptrs for the StepPointMCs
  art::Ptr<StepPointMC> newTriggerStepPtr[StrawEnd::nends];
  for(int i_end=0;i_end<StrawEnd::nends;++i_end){
    StrawEnd::End end = static_cast<StrawEnd::End>(i_end);

    const art::Ptr<StepPointMC>& old_step_point = old_straw_digi_mc.stepPointMC(end);
    if (old_step_point.isAvailable()) {
      newTriggerStepPtr[i_end] = copyStepPointMC( *old_step_point );
    }
  }
  
  std::vector<art::Ptr<StepPointMC> > newWaveformStepPtrs;
  for (const auto& i_step_mc : old_straw_digi_mc.stepPointMCs()) {
    if (i_step_mc.isAvailable()) {
      newWaveformStepPtrs.push_back(copyStepPointMC(*i_step_mc));
    }
  }
  
  StrawDigiMC new_straw_digi_mc(old_straw_digi_mc, newTriggerStepPtr, newWaveformStepPtrs); // copy everything except the Ptrs from the old StrawDigiMC  
  _newStrawDigiMCs->push_back(new_straw_digi_mc);
}

art::Ptr<mu2e::StepPointMC> mu2e::CompressMCTrkCollections::copyStepPointMC(const mu2e::StepPointMC& old_step) {

  _simParticlesToKeep[old_step.simParticle().id()].push_back(old_step.simParticle()->id());
  art::Ptr<SimParticle> newSimPtr(_newSimParticlesPID[old_step.simParticle().id()], old_step.simParticle()->id().asUint(), _newSimParticleGetter[old_step.simParticle().id()]);

  // Also need to add all the parents (and change their genParticles) too
  art::Ptr<SimParticle> childPtr = old_step.simParticle();
  art::Ptr<SimParticle> parentPtr = childPtr->parent();
  
  while (parentPtr) {
    _simParticlesToKeep[old_step.simParticle().id()].push_back(parentPtr->id());

    childPtr = parentPtr;
    parentPtr = parentPtr->parent();
  }
  
  StepPointMC new_step(old_step);
  new_step.simParticle() = newSimPtr;
  
  _newStepPointMCs->push_back(new_step);
  
  return art::Ptr<StepPointMC>(_newStepPointMCsPID, _newStepPointMCs->size()-1, _newStepPointMCGetter);
}



DEFINE_ART_MODULE(mu2e::CompressMCTrkCollections)
