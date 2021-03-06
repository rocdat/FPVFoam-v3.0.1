/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011-2015 OpenFOAM Foundation
     \\/     M anipulation  | Copyright (C) 2016 OpenCFD Ltd.
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

\*---------------------------------------------------------------------------*/

#include "maxDeltaxyz.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace LESModels
{
    defineTypeNameAndDebug(maxDeltaxyz, 0);
    addToRunTimeSelectionTable(LESdelta, maxDeltaxyz, dictionary);
}
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void Foam::LESModels::maxDeltaxyz::calcDelta()
{
    const fvMesh& mesh = turbulenceModel_.mesh();

    label nD = mesh.nGeometricD();

    tmp<volScalarField> hmax
    (
        new volScalarField
        (
            IOobject
            (
                "hmax",
                mesh.time().timeName(),
                mesh,
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            mesh,
            dimensionedScalar("zrero", dimLength, 0.0)
        )
    );

    const cellList& cells = mesh.cells();

    forAll(cells,cellI)
    {
        scalar deltaMaxTmp = 0.0;
        const labelList& cFaces = mesh.cells()[cellI];
        const point& centrevector = mesh.cellCentres()[cellI];

        forAll(cFaces, cFaceI)
        {
            label faceI = cFaces[cFaceI];
            const point& facevector = mesh.faceCentres()[faceI];
            scalar tmp = mag(facevector - centrevector);
            if (tmp > deltaMaxTmp)
            {
                deltaMaxTmp = tmp;
            }
        }
        hmax()[cellI] = deltaCoeff_*deltaMaxTmp;
    }

    if (nD == 3)
    {
        delta_.internalField() = hmax();
    }
    else if (nD == 2)
    {
        WarningInFunction
            << "Case is 2D, LES is not strictly applicable\n"
            << endl;

        delta_.internalField() = hmax();
    }
    else
    {
        FatalErrorInFunction
            << "Case is not 3D or 2D, LES is not applicable"
            << exit(FatalError);
    }

    // Handle coupled boundaries
    delta_.correctBoundaryConditions();
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::LESModels::maxDeltaxyz::maxDeltaxyz
(
    const word& name,
    const turbulenceModel& turbulence,
    const dictionary& dict
)
:
    LESdelta(name, turbulence),
    deltaCoeff_
    (
        dict.subDict(type() + "Coeffs").lookupOrDefault<scalar>("deltaCoeff", 1)
    )
{
    calcDelta();
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::LESModels::maxDeltaxyz::read(const dictionary& dict)
{
    const dictionary& coeffsDict(dict.subDict(type() + "Coeffs"));

    coeffsDict.readIfPresent<scalar>("deltaCoeff", deltaCoeff_);

    calcDelta();
}


void Foam::LESModels::maxDeltaxyz::correct()
{
    if (turbulenceModel_.mesh().changing())
    {
        calcDelta();
    }
}


// ************************************************************************* //
