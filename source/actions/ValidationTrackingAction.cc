// ----------------------------------------------------------------------------
// nexus | ValidationTrackingAction.cc
//
// This class is based on DefaultTrackinAction.
// In addition, it creates and saves ROOT histograms with the energy
// of the gammas produced in the simulation. Its purpose is to produce
// histograms to be compared across different versions of GEANT4,
// before changing version.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "ValidationTrackingAction.h"

#include "Trajectory.h"
#include "TrajectoryMap.h"
#include "IonizationElectron.h"

#include <G4Track.hh>
#include <G4TrackingManager.hh>
#include <G4Trajectory.hh>
#include <G4ParticleDefinition.hh>
#include <G4OpticalPhoton.hh>
#include <G4Gamma.hh>

#include <TFile.h>
#include <TH1F.h>


using namespace nexus;



ValidationTrackingAction::ValidationTrackingAction(): G4UserTrackingAction()
{
  gamma_energy_ = new TH1F("GammaEnergy", "GammaEnergy", 10000, 0, 4000.);
  gamma_energy_->GetXaxis()->SetTitle("Energy (keV)");
  gamma_energy_->GetYaxis()->SetTitle("Entries");
}



ValidationTrackingAction::~ValidationTrackingAction()
{
  out_file_ = new TFile("GammaEnergy.root", "recreate");
  gamma_energy_->Write();
  out_file_->Close();
}



void ValidationTrackingAction::PreUserTrackingAction(const G4Track* track)
{
  // Do nothing if the track is an optical photon or an ionization electron
  if (track->GetDefinition() == G4OpticalPhoton::Definition() ||
      track->GetDefinition() == IonizationElectron::Definition()) {
      fpTrackingManager->SetStoreTrajectory(false);
      return;
  }

  if (track->GetDefinition() == G4Gamma::Definition()) {
    gamma_energy_->Fill(track->GetKineticEnergy()/CLHEP::keV);
  }

  // Create a new trajectory associated to the track.
  // N.B. If the processesing of a track is interrupted to be resumed
  // later on (to process, for instance, its secondaries) more than
  // one trajectory associated to the track will be created, but
  // the event manager will merge them at some point.
  G4VTrajectory* trj = new Trajectory(track);

   // Set the trajectory in the tracking manager
  fpTrackingManager->SetStoreTrajectory(true);
  fpTrackingManager->SetTrajectory(trj);
 }



void ValidationTrackingAction::PostUserTrackingAction(const G4Track* track)
{
  // Do nothing if the track is an optical photon or an ionization electron
  if (track->GetDefinition() == G4OpticalPhoton::Definition() ||
    track->GetDefinition() == IonizationElectron::Definition()) return;

  Trajectory* trj = (Trajectory*) TrajectoryMap::Get(track->GetTrackID());

  // Do nothing if the track has no associated trajectory in the map
  if (!trj) return;

  // Record final time and position of the track
  trj->SetFinalPosition(track->GetPosition());
  trj->SetFinalTime(track->GetGlobalTime());
  trj->SetTrackLength(track->GetTrackLength());
  trj->SetFinalVolume(track->GetVolume()->GetName());
  trj->SetFinalMomentum(track->GetMomentum());

  // Record last process of the track
  G4String proc_name = track->GetStep()->GetPostStepPoint()->GetProcessDefinedStep()->GetProcessName();
  trj->SetFinalProcess(proc_name);
}
