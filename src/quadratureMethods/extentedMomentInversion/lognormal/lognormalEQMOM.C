/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2014-2015 Alberto Passalacqua
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is derivative work of OpenFOAM.

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

#include "lognormalEQMOM.H"
#include "scalar.H"
#include "scalarMatrices.H"
#include "constants.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(lognormalEQMOM, 0);

    addToRunTimeSelectionTable
    (
        extendedMomentInversion, 
        lognormalEQMOM, 
        dictionary
    ); 
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::lognormalEQMOM::lognormalEQMOM
(
    const dictionary& dict,
    const label nMoments,    
    const label nSecondaryNodes
)
:
    extendedMomentInversion(dict, nMoments, nSecondaryNodes)
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::lognormalEQMOM::~lognormalEQMOM()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //
Foam::scalar Foam::lognormalEQMOM::secondaryAbscissa
(
    scalar primaryAbscissa,
    scalar secondaryAbscissa,
    scalar sigma
)
{
    return primaryAbscissa*secondaryAbscissa;
}

void Foam::lognormalEQMOM::momentsStarToMoments
(
    scalar sigma,
    univariateMomentSet& moments,
    const univariateMomentSet& momentsStar
)
{
    scalar z = exp(sqr(sigma)/2.0);

    forAll(moments, momentI)
    {
        moments[momentI] = momentsStar[momentI]*pow(z, momentI*momentI);
    }
}

void Foam::lognormalEQMOM::momentsToMomentsStar
(
    scalar sigma,
    const univariateMomentSet& moments,
    univariateMomentSet& momentsStar
)
{
    scalar z = exp(-sqr(sigma)/2.0);

    forAll(moments, momentI)
    {
        momentsStar[momentI] = moments[momentI]*pow(z, momentI*momentI);
    }
}

Foam::scalar Foam::lognormalEQMOM::m2N
(
    scalar sigma, 
    univariateMomentSet momentsStar
)
{   
    if (momentsStar.nRealizableMoments() >= momentsStar.size() - 1)
    {
        scalar z = exp(sqr(sigma)/2.0);
        label nNod = momentsStar.size() - 1;
        
        return momentsStar.last()*pow(z, nNod*nNod);
    }
    
    return GREAT;
}

void Foam::lognormalEQMOM::recurrenceRelation
(
    scalarDiagonalMatrix& a, 
    scalarDiagonalMatrix& b,
    scalar primaryAbscissa,
    scalar sigma
)
{
    scalar eta = exp(sqr(sigma)/2.0);
    scalar sqEta = sqr(eta);
    
    a[0] = eta;
    
    for (label aI = 1; aI < a.size(); aI++)
    {
        a[aI] = ((sqEta + 1)*pow(sqEta, scalar(aI)) - 1.0)
                *pow(eta, 2.0*scalar(aI) - 1.0);
    }

    b[0] = 0.0;

    for (label bI = 1; bI < b.size(); bI++)
    {
        b[bI] = pow(eta, 6.0*scalar(bI) - 4.0)*(pow(sqEta, scalar(bI)) - 1.0);
    }
}

Foam::scalar Foam::lognormalEQMOM::sigmaMax(univariateMomentSet& moments)
{  
    label nRealizableMoments = moments.nRealizableMoments();
    
    scalar sigmaZeta1 = 
            sqrt(2.0*log(sqrt(moments[0]*moments[2]/(sqr(moments[1])))));

    if (nRealizableMoments > 3)
    {
        scalar sigmaZeta2 = 
            sqrt(2.0*log(sqrt(moments[1]*moments[3]/(sqr(moments[2])))));

        return min(sigmaZeta1, sigmaZeta2);
    }

    return sigmaZeta1;
}

// ************************************************************************* //
