/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

Application
    fireFoam

Description
    Transient PIMPLE solver for Fires and turbulent diffusion flames with
    reacting Lagrangian parcels, surface film and pyrolysis modelling.

\*---------------------------------------------------------------------------*/

#include "mapDistribute.H"
#include "fvCFD.H"
#include "turbulenceModel.H"
#include "basicReactingCloud.H"
#include "surfaceFilmModel.H"
#include "pyrolysisModelCollection.H"
#include "radiationModel.H"
#include "SLGThermo.H"
#include "hsCombustionThermo.H"
#include "solidChemistryModel.H"
#include "combustionModel.H"
#include "pimpleControl.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    #include "setRootCase.H"

    #include "createTime.H"
    #include "createMesh.H"
    #include "readChemistryProperties.H"
    #include "readGravitationalAcceleration.H"
    #include "createFields.H"
    #include "createClouds.H"
    #include "createSurfaceFilmModel.H"
    #include "createPyrolysisModel.H"
    #include "createRadiationModel.H"
    #include "initContinuityErrs.H"
    #include "readTimeControls.H"
    #include "compressibleCourantNo.H"
    #include "setInitialDeltaT.H"
    #include "readPyrolysisTimeControls.H"

    pimpleControl pimple(mesh);

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    Info<< "\nStarting time loop\n" << endl;

    while (runTime.run())
    {
        #include "readTimeControls.H"
        #include "compressibleCourantNo.H"
        #include "solidRegionDiffusionNo.H"
        #include "setMultiRegionDeltaT.H"
        #include "setDeltaT.H"

        runTime++;

        Info<< "Time = " << runTime.timeName() << nl << endl;

        parcels.evolve();

        surfaceFilm.evolve();

        pyrolysis.evolve();

        if (solvePrimaryRegion)
        {
            #include "rhoEqn.H"

            // --- PIMPLE loop
            for (pimple.start(); pimple.loop(); pimple++)
            {
                #include "UEqn.H"
                #include "YhsEqn.H"

                // --- PISO loop
                for (int corr=1; corr<=pimple.nCorr(); corr++)
                {
                    #include "pEqn.H"
                }

                if (pimple.turbCorr())
                {
                    turbulence->correct();
                }
            }

            rho = thermo.rho();
        }

        runTime.write();

        Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
            << "  ClockTime = " << runTime.elapsedClockTime() << " s"
            << nl << endl;
    }

    Info<< "End" << endl;

    return(0);
}


// ************************************************************************* //
