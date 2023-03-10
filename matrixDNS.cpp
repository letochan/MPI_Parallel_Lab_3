#include "mpi.h"

#include <iostream>
#include <random>
#include <chrono>
#define maxNtoView 5

using namespace std;

long int *cs(long int *array,int n) //сдвиг
{
	long int tmp = array[0];
	for (int i = 0; i < n - 1; i++)
		array[i] = array[i + 1];
	array[n - 1] = tmp;
	return array;
}

long int **matrix(int rows, int cols, bool full)   //создаем матрицу
{
	long int *data = new long int[rows*cols];
	long int **array = new long int*[rows];
	for (int i = 0; i < rows; i++)
		array[i] = &(data[cols*i]);

	for (int i = 0; i < rows; i++)
		for (int j = 0; j < cols; j++)
		{
			if (full)
			{
				array[i][j] = rand() % (10);
			}

			else array[i][j] = 0;
		}
	return array;
}


long int **transport(long int** matr, int n)
{
	long int **array = matrix(n, n, false);
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
				array[i][j] = matr[j][i];
	return array;
}

int main(int argc, char *argv[])
{

	srand(time(0));
	int k = 0;
	int n=-1;
	bool kom = true;
	long int** matrA;//задание матрицы A
	long int** matrB; //задание матрицы B
	long int** matrC; //результирующая матрица
	long int** tmp; //для подсчёта матрица
	long int** tmp1; //для подсчёта матрица 2
	long int** tmpres; //для подсчёта матрица 3
	int length2 = 0;
	int length = 0;
	bool view = true;

	bool compare = true;   //сравнение результатов
	int numOfProc;   //число процессов
	int rankOfProc;   //ранг процесса

	double start_time, end_time;
	MPI_Init(&argc, &argv);
	MPI_Status status;
	MPI_Comm_size(MPI_COMM_WORLD, &numOfProc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rankOfProc);

	if (rankOfProc == 0)
	{
		while (((n <= 0))/* || ((n % numOfProc != 0))*/)
		{
			cout << "\nEnter the size of quadratnaya matix: \n";
			cin >> n;
		}
	}
	
	MPI_Barrier(MPI_COMM_WORLD); //ВОТ ОН БАРЬЕР БЛЯДСКИЙ НЕ РАБОТАЕТ


	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);


		length = n*n / numOfProc;
		length2 = n;

	//ширина блока
	int toView = n;

	if (n > maxNtoView)
		toView = maxNtoView;

	matrA = matrix(n,n, true); //задаем матрицы
	matrB = matrix(n,n, true);
	matrC = matrix(n,n, false);
	tmp = matrix(length, length2, false);
	tmp1 = matrix( length, length2, false);
	tmpres = matrix(length, length, false);
	

	if (rankOfProc == 0)
	{
		if (n >= maxNtoView)
			cout << "\nMatrix too large to view it. You will to see first " << maxNtoView << " elements\n";

		cout << "Matrix A: \n";
		for (int i = 0; i < toView; i++)
		{
			for (int j = 0; j < toView; j++)
			{
				cout << matrA[i][j] << " "; //если матрица маленькая - выводим
			}
			cout << "\n";
		}

		cout << "Matrix B: \n";
		for (int i = 0; i < toView; i++)
		{
			for (int j = 0; j < toView; j++)
			{
				cout << matrB[i][j] << " "; //если матрица маленькая - выводим
			}
			cout << "\n";
		}
		matrB = transport(matrB, n); //транспонируем для удобства
		cout << "trMatrix B: \n";
		for (int i = 0; i < toView; i++)
		{
			for (int j = 0; j < toView; j++)
			{
				cout << matrB[i][j] << " "; //если матрица маленькая - выводим
			}
			cout << "\n";
		}
	}


	MPI_Barrier(MPI_COMM_WORLD);
	
	MPI_Scatter(*matrA, n*n,MPI_INT,*tmp,n*length,MPI_INT,0,MPI_COMM_WORLD); 

	MPI_Scatter(*matrB, n*n, MPI_INT, *tmp1, n*length, MPI_INT, 0, MPI_COMM_WORLD);
	

	if (view)
	{
		start_time = MPI_Wtime();	//пуск таймера
		view = false;
	}


	for (int i = 0; i < length; i++) 
	{
		for (int j = 0; j < length; j++)
			for (int k = 0; k < n; k++)
			{
				tmpres[i][j] +=  tmp[i][k] * tmp1[j][k];
			}
	}

	//for (int i = 0; i < length; i++) 
	//{
	//	for (int j = 0; j < length; j++)
	//		for (int k = 0; k < n; k++)
	//		{
	//			tmpres[i][j] += tmp[i][0] * tmp1[j][0];
	//			tmp [i] = cs(tmp[i]);
	//          tmp[i] = cs(tmp1[j]);
	//		}
	//}

		
	MPI_Gather(*tmpres, length*length, MPI_INT, *matrC, n*n, MPI_INT, 0, MPI_COMM_WORLD);

	if (rankOfProc == 0)
	{
		
		end_time = MPI_Wtime();
		cout << "multiplied matrix is:\n";
		for (int i = 0; i < toView; i++)
		{
	
			for (int j = 0; j < toView; j++)
			{
				cout << matrC[i][j] << " "; //если матрица маленькая - выводим
	
			}
			cout << "\n";
		}
		cout << "Time of running = " << end_time - start_time;
	}
	MPI_Barrier(MPI_COMM_WORLD);

free(matrA);
free(matrB);
free(matrC);
free(tmp);
MPI_Finalize();

return 0;
}
