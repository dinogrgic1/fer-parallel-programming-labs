// 4 u nizu - glavni program
#include<iostream>
#include<ctime>
using namespace std;
#include"board.h"	// razred za igracu plocu

const int DEPTH = 6;	// default dubina stabla

double Evaluate(Board Current, data LastMover, int iLastCol, int iDepth);

int main(int argc, char **argv)
{
	Board B;
	double dResult, dBest;
	int iBestCol, iDepth = DEPTH;
	if(argc<2)
	{	cout << "Uporaba: <program> <fajl s trenutnim stanjem> [<dubina>]" << endl;
		return 0;
	}
	B.Load(argv[1]);
	if(argc>2)
		iDepth = atoi(argv[2]);
	srand( (unsigned)time( NULL ) );
	// provjerimo jel igra vec gotova (npr. ako je igrac pobijedio)
	for(int iCol=0; iCol<B.Columns(); iCol++)
		if(B.GameEnd(iCol))
		{	cout << "Igra zavrsena!" << endl;
			return 0;
		}
	// pretpostavka: na potezu je CPU
	do
	{	cout << "Dubina: " << iDepth << endl;
		dBest = -1; iBestCol = -1;
		for(int iCol=0; iCol<B.Columns(); iCol++)
		{	if(B.MoveLegal(iCol))
			{	if(iBestCol == -1)
					iBestCol = iCol;
				B.Move(iCol, CPU);
				dResult = Evaluate(B, CPU, iCol, iDepth-1);
				B.UndoMove(iCol);
				if(dResult > dBest || (dResult == dBest && rand()%2 == 0))
				{	dBest = dResult;
					iBestCol = iCol;
				}
				cout << "Stupac " << iCol << ", vrijednost: " << dResult << endl;
			}
		}
		iDepth /= 2;	
	// zasto petlja? ako svi potezi vode u poraz, racunamo jos jednom za duplo manju dubinu
	// jer igrac mozda nije svjestan mogucnosti pobjede
	}while(dBest == -1 && iDepth > 0);
	cout << "Najbolji: " << iBestCol << ", vrijednost: " << dBest << endl;
	B.Move(iBestCol, CPU);
	B.Save(argv[1]);
	// jesmo li pobijedili
	for(int iCol=0; iCol<B.Columns(); iCol++)
		if(B.GameEnd(iCol))
		{	cout << "Igra zavrsena! (pobjeda racunala)" << endl;
			return 0;
		}
	return 0;
}

// rekurzivna funkcija: ispituje sve moguce poteze i vraca ocjenu dobivenog stanja ploce
// Current: trenutno stanje ploce
// LastMover: HUMAN ili CPU
// iLastCol: stupac prethodnog poteza
// iDepth: dubina se smanjuje do 0
double Evaluate(Board Current, data LastMover, int iLastCol, int iDepth)
{
	double dResult, dTotal;
	data NewMover;
	bool bAllLose = true, bAllWin = true;
	int iMoves;
	
	if(Current.GameEnd(iLastCol))	// igra gotova?
		if(LastMover == CPU)
			return 1;	// pobjeda
		else //if(LastMover == HUMAN)
			return -1;	// poraz
	// nije gotovo, idemo u sljedecu razinu
	if(iDepth == 0)
		return 0;	// a mozda i ne... :)
	iDepth--;
	if(LastMover == CPU)	// tko je na potezu
		NewMover = HUMAN;
	else
		NewMover = CPU;
	dTotal = 0;
	iMoves = 0;	// broj mogucih poteza u ovoj razini
	for(int iCol=0; iCol<Current.Columns(); iCol++)
	{	if(Current.MoveLegal(iCol))	// jel moze u stupac iCol
		{	iMoves++;
			Current.Move(iCol, NewMover);
			dResult = Evaluate(Current, NewMover, iCol, iDepth);
			Current.UndoMove(iCol);
			if(dResult > -1)
				bAllLose = false;
			if(dResult != 1)
				bAllWin = false;
			if(dResult == 1 && NewMover == CPU)
				return 1;	// ako svojim potezom mogu doci do pobjede (pravilo 1)
			if(dResult == -1 && NewMover == HUMAN)
				return -1;	// ako protivnik moze potezom doci do pobjede (pravilo 2)
			dTotal += dResult;
		}
	}
	if(bAllWin == true)	// ispitivanje za pravilo 3.
		return 1;
	if(bAllLose == true)
		return -1;
	dTotal /= iMoves;	// dijelimo ocjenu s brojem mogucih poteza iz zadanog stanja
	return dTotal;
}
