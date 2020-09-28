/*
/Name of Program: stats
/Coder: Jean-Luc Desjardins
/Date: Feb 28 2020
/Version: 1.0
/Purpose: reads number pairs from file or keyboard then calculates minimum value, maximum value, 
/median value, arithmetic mean, mean absolute deviation – (mean, median, mode),
/variance (of a discrete random variable), standard deviation (of a finite population), 
/mode, least squares regression line and outliers(2x and 3x)
*/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

typedef double dataSetType;
char* getDynamicSize(FILE* stream);
int countCommas(char input[]);
void fillArray(char input[], dataSetType xArray[], dataSetType yArray[]);
void swap(dataSetType* pa, dataSetType* pb);
dataSetType calcMedian(dataSetType array[], int n);
dataSetType calcMean(dataSetType array[], int n);
dataSetType calcMax(dataSetType array[], int n);
dataSetType calcMin(dataSetType array[], int size);
dataSetType calcMeanAbsoluteDeviation(dataSetType arr[], int n);
dataSetType calcVariance(dataSetType array[], int n);
dataSetType calcDeviation(dataSetType variance);
dataSetType calcAbsDeviation(dataSetType* values, int length, dataSetType data_point);
int partitionParallel(dataSetType main_arr[], dataSetType parallel_arr[], int low, int high);
void quickSortParallel(dataSetType arr[], dataSetType parallel_arr[], int low, int high);
int calculateMode(dataSetType* values, int length, dataSetType* mode);
void calcMode(dataSetType arrayX[], dataSetType arrayY[], int length, dataSetType modeX, dataSetType modeY);
void calcModeDeviation(dataSetType arrayX[], dataSetType arrayY[], int length, dataSetType modeX, dataSetType modeY);
void calcAB(dataSetType* xArray, dataSetType* yArray, int length, dataSetType* a, dataSetType* b);
dataSetType calcX(dataSetType array[], int numComma);
dataSetType calcY(dataSetType a, dataSetType b, dataSetType x);
void calcOutliers(dataSetType xArray[], dataSetType yArray[], int length, dataSetType meanY, dataSetType stdDevArrayY);

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

int main(int argc, char* argv[]) {
	
	FILE* stream = stdin;
	//too many arguments 
	if (argc > 2)
	{
		printf("Error, too many args %d \n", argc);
		return EXIT_FAILURE;
	}
	//correct arguments
	else if (argc == 2)
	{
		stream = fopen(argv[1], "r");
		//check for file opening failure
		if (stream == NULL)
		{
			printf("Error %s ", argv[1]);
			return EXIT_FAILURE;
		}
	}
	printf("stats (1.0), Jean-Luc Desjardins\n");
	printf("Enter a list of comma-separated real number pairs terminated by EOF or non numeric input.\n");
	//get input from user or file
	char* input = getDynamicSize(stream);
	
	//
	fclose(stream);
	//count the number of commas in the input to size the arrays
	int numComma = countCommas(input);

	printf("Results:\n-------------------------------------------------------------------\n");
	if (numComma == 0)
	{
		printf("%-10s\t\t\t%10s\t", "# elements", "no samples");
		return EXIT_SUCCESS;
	}
	printf("%-10s\t\t\t%10d\t\t%10d\n", "# elements", numComma, numComma);
			//create arrays based on the number of commas 
	dataSetType* xArray = (dataSetType*)malloc(numComma * sizeof(dataSetType));
	dataSetType* yArray = (dataSetType*)malloc(numComma * sizeof(dataSetType));
	//take char array named input and split it into tokens, assign tokens to proper array
	fillArray(input, xArray, yArray);
	//we no longer need input array
	free(input);
	dataSetType a, b;
	calcAB(xArray, yArray, numComma, &a, &b);
	//sort y int parallel and do calculations
	quickSortParallel(yArray, xArray, 0, numComma - 1);
	dataSetType minY = calcMin(yArray, numComma);
	dataSetType maxY = calcMax(yArray, numComma);
	dataSetType meanY = calcMean(yArray, numComma);
	dataSetType medianY = calcMedian(yArray, numComma);
	dataSetType varianceY = calcVariance(yArray, numComma);
	dataSetType devY = calcDeviation(calcVariance(yArray, numComma));
	dataSetType abMeanY = calcMeanAbsoluteDeviation(yArray, numComma);
	dataSetType abMedianY = calcAbsDeviation(yArray, numComma, medianY);

	//sort x in parallel then calculate all the variables
	quickSortParallel(xArray, yArray, 0, numComma - 1);
	dataSetType minX = calcMin(xArray, numComma);
	dataSetType maxX = calcMax(xArray, numComma);
	dataSetType meanX = calcMean(xArray, numComma);
	dataSetType medianX = calcMedian(xArray, numComma);
	dataSetType varianceX = calcVariance(xArray, numComma);
	dataSetType devX = calcDeviation(calcVariance(xArray, numComma));
	dataSetType abMeanX = calcMeanAbsoluteDeviation(xArray, numComma);
	dataSetType abMedianX = calcAbsDeviation(xArray, numComma, medianX);
	dataSetType x = calcX(xArray, numComma);
	//print results 
	printf("%-10s\t\t\t%10.3lf\t\t%10.3lf\n", "minimum", minX, minY);
	printf("%-10s\t\t\t%10.3lf\t\t%10.3lf\n", "maximum", maxX, maxY);
	printf("%-10s\t\t\t%10.3lf\t\t%10.3lf\n", "mean", meanX, meanY);
	printf("%-10s\t\t\t%10.3lf\t\t%10.3lf\n", "median", medianX, medianY);
	dataSetType modeX = 0;
	dataSetType modeY = 0;
	calcMode(xArray, yArray, numComma, modeX, modeY);
	printf("%-10s\t\t\t%10.3lf\t\t%10.3lf\n", "variance", varianceX, varianceY);
	dataSetType stdDevArrayY = calcDeviation(calcVariance(yArray, numComma));
	printf("%-10s\t\t\t%10.3lf\t\t%10.3lf\n", "std. dev.", devX, stdDevArrayY);
	printf("mean absolute deviations:\n");
	printf("%-19s\t\t%10.3lf\t\t%10.3lf\n", "-> about the mean", abMeanX, abMeanY);
	printf("%-19s\t\t%10.3lf\t\t%10.3lf\n", "-> about the median", abMedianX, abMedianY);
	calcModeDeviation(xArray, yArray, numComma, modeX, modeY);
	printf("%-28s%6s%8.3lf\t%10s%8.3lf\n", "regression line", "a = ", a, "b = ", b);

	dataSetType y = calcY(a, b, x);
	printf("%-31s%4s%7.3lf\t%10s%8.3lf\n", "Y at mid(X)", "x = ", x, "y = ", y);
	calcOutliers(xArray, yArray, numComma, meanY, stdDevArrayY);

	//free allocated memory
	free(xArray);
	free(yArray);
}

/*! \fn char* getDynamicSize()
	 \returns an array of char that is the input of the file or keyboard
	 \param input [in] file*stream
	 \purpose this function takes input from the keyboard or the file and stores it
 */
char* getDynamicSize(FILE*stream) {
	//eat the leading whitespace
	int ch;
	while ((ch = getc(stream)) != EOF && isspace(ch));
	//return null if only whitespace 
	if (EOF == ch)
		return NULL;
	//create starting buffer
	size_t capacity = 4;
	size_t size = 1;
	char* input = (char*)malloc(capacity + 1);
	if (NULL == input)
		return NULL;
	//put char into buffer
	input[0] = ch;
	//keep grabing characters and append the buffer (It will expand if required)
	while ((ch = getc(stream)) != EOF ) {
		// If char is alphabetic then break out of the loop
		if (isalpha(ch)) {
			break;
		}
		//if it's full then double the array
		if (capacity == size) {
			char* inputDoubled = (char*)realloc(input, (capacity *= 2) + 1);
			if (NULL == inputDoubled) {
				free(input);
				return NULL;
			}
			input = inputDoubled;
		}
		input[size++] = ch;
	}
	//if not EOF return the whitespace to stream
	if (EOF != ch)
		ungetc(ch, stream);
	//store the terminating 0
	input[size] = 0;
	//return the char array
	return input;
}


/*! \fn int countCommas()
	 \returns an integer that is the numbers of commas in the array
	 \param input [in] a char array
	 \purpose this function will count the number of ',' in the array 
 */
int countCommas(char input[]) {
	int count = 0;
	for (int i = 0; input[i]; i++) {
		if (input[i] == ',') {
			count++;
		}
	}
	return count;
}

/*! \fn void fillArray()
	 \returns void (fills the dataSetType arrays)
	 \param input [in] a char array and two dataSetType arrays to be filled
	 \purpose this function take the input and fill two arrays with dataSetType
 */
void fillArray(char input[], dataSetType xArray[], dataSetType yArray[]) {
	char seps[] = " ,\t\n";
	char* token1 = NULL;
	char* next_token1 = NULL;

	//establish string and get the first token:
	token1 = strtok_s(input, seps, &next_token1);
	int i = 0;
	int x = 0, y = 0;
	//while there are tokens in "input"
	while (token1 != NULL)
	{
		if (i % 2 == 0) {
			xArray[x] = atof(token1);
			++x;
		}
		else {
			yArray[y] = atof(token1);
			++y;
		}
		i++;
		token1 = strtok_s(NULL, seps, &next_token1);
	}
}
/*! \fn void swap()
	 \returns nothing (void)
	 \param input [in] two memory locations
	 \purpose this function will swap the contents of two memory locations
 */
void swap(dataSetType* pa, dataSetType* pb) {
	dataSetType hold = *pa;
	*pa = *pb;
	*pb = hold;
}


/*! \fn dataSetType calcMedian()
	 \returns dataSetType that is the median of the array
	 \param input [in] a dataSetType array and an int which is it's length
	 \purpose this function will calculate the median
 */
dataSetType calcMedian(dataSetType array[], int n) {
	dataSetType median = 0;

	// If numbers are even
	if (n % 2 == 0)
		median = (array[(n - 1) / 2] + array[n / 2]) / 2.0;
	// Number of elements are odd
	else
		median = array[n / 2];

	return median;
}
/*! \fn dataSetType calcMean()
	 \returns dataSetType that is the mean of the array
	 \param input [in] a dataSetType array and an int which is it's length 
	 \purpose this function will calculate the mean of the array
 */
dataSetType calcMean(dataSetType array[], int n) {
	dataSetType sum = 0;
	for (int i = 0; i < n; i++) {
		sum += array[i];
	}
	sum /= n;
	return sum;
}
/*! \fn dataSetType calcMax()
	 \returns dataSetType that is the max of the array
	 \param input [in] a dataSetType array and an int which is it's length
	 \purpose this function will locate the maximum cell in the array
 */
dataSetType calcMax(dataSetType array[], int n) {
	dataSetType maximum = array[0];
	for (int i = 1; i < n; i++)
	{
		if (array[i] > maximum)
		{
			maximum = array[i];
		}
	}
	return maximum;
}

/*! \fn dataSetType calcMin()
	 \returns dataSetType that is the min of the array
	 \param input [in] a dataSetType array and an int which is it's length
	 \purpose this function will located the min element of the array
 */
dataSetType calcMin(dataSetType array[], int size) {

	dataSetType minimum = array[0];
	for (int i = 1; i < size; i++)
	{
		if (array[i] < minimum)
		{
			minimum = array[i];
		}
	}
	return minimum;
}

/*! \fn dataSetType calcMeanAbsoluteDeviation()
	 \returns dataSetType that is the mean absolute deviation of the array
	 \param input [in] a dataSetType array and an int which is it's length
	 \purpose this function will located the mean absolute deviation element of the array
 */
dataSetType calcMeanAbsoluteDeviation(dataSetType arr[], int n)
{
	//calculate the sum of absolute 
	//deviation about mean. 
	dataSetType absSum = 0;
	for (int i = 0; i < n; i++) {
		absSum += fabs(arr[i] - calcMean(arr, n));
	}
	//return mean absolute deviation about mean. 
	return absSum / n;
}
/*! \fn dataSetType calcVariance()
	 \returns dataSetType that is the variancen of the array
	 \param input [in] a dataSetType array and an int which is it's length
	 \purpose this function will located the mean absolute deviation element of the array
 */
dataSetType calcVariance(dataSetType array[], int n) {
	dataSetType mean, variance, Sum = 0, Differ, varSum = 0;
	for (int i = 0; i < n; i++)
	{
		Sum = Sum + array[i];
	}
	mean = Sum / (dataSetType)n;
	for (int i = 0; i < n; i++)
	{
		Differ = array[i] - mean;
		varSum = varSum + pow(Differ, 2);
	}
	variance = varSum / (dataSetType)n;
	return variance;
}

/*! \fn dataSetType calcDeviation()
	 \returns a dataSetType
	 \param input [in] dataSetType holding the variance value
	 \purpose this function will calculate the deviation
 */
dataSetType calcDeviation(dataSetType variance) {
	return sqrt(variance);
}

/*! \fn dataSetType calcAbsDeviation()
	 \returns a dataSetType which represents the absolute deviation
	 \param input [in] an array of dataSetType, an int which is the 
	 \length and a data point which will be either mean or median or mode 
	 \purpose this function will calculate the absolute deviation of mean median or mode
 */
dataSetType calcAbsDeviation(dataSetType* values, int length, dataSetType data_point)
{
	dataSetType sum = 0;
	for (int i = 0; i < length; i++)
	{
		dataSetType diff = values[i] - data_point;
		sum += fabs(diff);
	}
	return sum / length;
}

/*! \fn dataSetType calcAbsDeviation()
	 \returns an int which represents the position in the array
	 \param input [in] two arrays of dataSetType, two ints 
	 \which are the high and low values
	 \purpose this function will sort two arrays in parallel 
 */
int partitionParallel(dataSetType main_arr[], dataSetType parallel_arr[], int low, int high)
{
	dataSetType pivot = main_arr[high];
	int i = (low - 1);
	for (int j = low; j <= high - 1; j++)
	{
		if (main_arr[j] <= pivot)
		{
			i++;
			//swap both arrays at the same place
			swap(&main_arr[i], &main_arr[j]);
			swap(&parallel_arr[i], &parallel_arr[j]);
		}
	}
	//swap both arrays at the same place
	swap(&main_arr[i + 1], &main_arr[high]);
	swap(&parallel_arr[i + 1], &parallel_arr[high]);
	return (i + 1);
}

/*! \fn dataSetType quickSortParallel()
	 \returns nothing (void) 
	 \param input [in] two arrays of dataSetType, two ints 
	 \which are the high and low values
	 \purpose this function will sort two arrays in parallel
 */
void quickSortParallel(dataSetType arr[], dataSetType parallel_arr[], int low, int high)
{
	if (low < high)
	{
		int pi = partitionParallel(arr, parallel_arr, low, high);
		quickSortParallel(arr, parallel_arr, low, pi - 1);
		quickSortParallel(arr, parallel_arr, pi + 1, high);
	}
}

/*! \fn dataSetType quickSortParallel()
	 \returns an int that is the frequency 
	 \param input [in] an array of data_type_set, an int
	 \that is the length of the array and a dataSetType
	 \which is the mode that will be calculated
	 \purpose this function will calculate mode and frequency 
 */
int calculateMode(dataSetType* values, int length, dataSetType* mode)
{
	dataSetType value;
	int num_occurances = 0;
	int max_occurances = 0;
	
	int i, y;
	for (i = 0; i < length; i = y)
	{
		int occurances = 0;

		for (y = i; y < length && values[i] == values[y]; y++)
		{
			occurances++;
		}
		//must adjust if occurances are greater than max
		if (occurances > max_occurances)
		{
			num_occurances = occurances;
			max_occurances = occurances;
			value = values[i];
		}
		//used to calculate no mode
		else if (occurances == max_occurances)
		{
			num_occurances = 0;
		}
	}

	if (num_occurances == 1 && length > 1)
		return 0;
	//returns the mode without returning
	if (num_occurances)
		*mode = value;
	return num_occurances;
}




/*! \fn dataSetType calcMode()
	 \returns nothing (void) 
	 \param input [in] two arrays of dataSetType, an int which is the
	 \length of the array, two dataSetType which are modes of x and y
	 \purpose this function will print calculate and print mode 
 */
void calcMode(dataSetType arrayX[], dataSetType arrayY[], int length, dataSetType modeX, dataSetType modeY) {
	//reason why we sort is because we calculate mode at the same time and both x and y need to be sorted
	int freqY = calculateMode(arrayY, length, &modeY);
	quickSortParallel(arrayX, arrayY, 0, length - 1);
	int freqX = calculateMode(arrayX, length, &modeX);
	quickSortParallel(arrayY, arrayX, 0, length - 1);

	if (freqY > 0 && freqX > 0) {
		printf("%-10s\t\t\tfreq.= %3d\t\tfreq.= %3d\n", "mode", freqX, freqY);
		printf("%-10s\t\t\t%10.3lf\t\t%10.3lf\n", " ", modeX, modeY);
	}
	else if (freqX > 0 && freqY == 0) {
		printf("%-10s\t\t\tfreq.= %3d\t\t%10s\n", "mode", freqY, "no mode");
		printf("%-10s\t\t\t%s%10.3fl\t\t%s\n", " ", " ", modeX, " ");
	}
	else if (freqY > 0 && freqX == 0) {
		printf("%-10s\t\t\t%10s\t\tfreq.= %3d\n", "mode", "no mode", freqY);
		printf("%-28s%6s\t\t%10s%8.3lf\n", "", "", "", modeY);
	}
	else {
		printf("%-10s\t\t\t%10s\t\t%10s\n", "mode", "no mode", "no mode");
	}
}


/*! \fn void calcModeDeviation()
	 \returns nothing (void)
	 \param input [in] two arrays of dataSetType, an int which is the
	 \length of the array, two dataSetType which are modes of x and y
	 \purpose this function will calculate and print mode deviation
 */
void calcModeDeviation(dataSetType arrayX[], dataSetType arrayY[], int length, dataSetType modeX, dataSetType modeY) {
	int freqY = calculateMode(arrayY, length, &modeY);
	quickSortParallel(arrayX, arrayY, 0, length - 1);
	int freqX = calculateMode(arrayX, length, &modeX);
	
	dataSetType xModeDeviation = calcAbsDeviation(arrayX, length, modeX);
	dataSetType yModeDeviation = calcAbsDeviation(arrayY, length, modeY);
	if (freqY > 0 && freqX > 0) {
		printf("%-19s\t\t%10.3lf\t\t%10.3lf\n", "-> about the mode", xModeDeviation, yModeDeviation);
	}
	else if (freqX > 0 && freqY == 0) {
		printf("%-10s\t\t%5.3f\t\t%10s\n", "-> about the mode", xModeDeviation, "no mode");
	}
	else if (freqY > 0 && freqX == 0) {
		printf("%-10s\t\t%10s\t\t%10.3f\n", "-> about the mode", "no mode", yModeDeviation);
	}
	else {
		printf("%-10s\t\t%10s\t\t%10s\n", "-> about the mode", "no mode", "no mode");
	}
}

/*! \fn void calcAB()
	 \returns nothing (void)
	 \param input [in] two arrays of dataSetType, an int which is the
	 \length of the array, two dataSetType which are the value of a and b
	 \purpose this function will calculate a and b
 */
void calcAB(dataSetType* xArray, dataSetType* yArray, int length, dataSetType* a, dataSetType* b)
{
	dataSetType x_mean = calcMean(xArray, length);
	dataSetType yMean = calcMean(yArray, length);
	dataSetType sumSquares = 0;
	dataSetType sumProducts = 0;

	for (int i = 0; i < length; i++)
	{
		dataSetType xDif = (xArray[i] - x_mean);
		dataSetType yDif = (yArray[i] - yMean);
		sumSquares += xDif * xDif;
		sumProducts += xDif * yDif;
	}

	*b = sumProducts / sumSquares;
	*a = yMean - (*b * x_mean);
}

/*! \fn void calcX()
	 \returns a dataSetType which is the value of X
	 \param input [in] an array of dataSetType, an int which is the
	 \length of the array
	 \purpose this function will calculate x
 */
dataSetType calcX(dataSetType array[], int numComma) {
	dataSetType x = ((array[numComma - 1] - array[0]) / 2) + array[0];
	return x;
}

/*! \fn void calcY()
	 \returns a dataSetType which is the value of X
	 \param input [in] an array of dataSetType, an int which is the
	 \length of the array
	 \purpose this function will calculate x
 */
dataSetType calcY(dataSetType a, dataSetType b, dataSetType x) {
	return (a + (b * x));
}

/*! \fn void calcOutliers()
	 \returns nothing (void)
	 \param input [in] two arrays of dataSetType, an int which is the
	 \length of the array, two dataSetType which are the mean and 
	 \standard deviation of y
	 \purpose this function will calculate and print outliers
 */
void calcOutliers(dataSetType xArray[], dataSetType yArray[], int length, dataSetType meanY, dataSetType stdDevArrayY) {
	dataSetType number;
	int threeXOutlier = 0;
	int twoXOutlier = 0;
	for (int i = 0; i < length; ++i) {
		number = yArray[i] - meanY;
		number = fabs(number);
		if (number > (stdDevArrayY*3)) {
			threeXOutlier++;
		}
		else if (number > (stdDevArrayY * 2)) {
			twoXOutlier++;
		}
	}
	//three time outliers are also two time outliers
	twoXOutlier += threeXOutlier;
	if (twoXOutlier > 0) {
		printf("%-26s%12s%3d\t\n", "Outliers(2x)", "# outliers = ", twoXOutlier);
		for (int i = 0; i < length; ++i) {
			number = yArray[i] - meanY;
			number = fabs(number);
			if (number > (stdDevArrayY * 2)) {
				printf("%-10s\t\t\t%10.3lf\t\t%10.3lf\n", "", xArray[i], yArray[i]);
			}
		}
	}
	else {
		printf("%-19s\t%18s\n", "Outliers(2x)", "no outliers");
	}
	if (threeXOutlier > 0) {
		printf("%-26s%12s%3d\t\n", "Outliers(3x)", "# outliers = ", threeXOutlier);
		for (int i = 0; i < length; ++i) {
			number = yArray[i] - meanY;
			number = fabs(number);
			if (number > (stdDevArrayY * 3)) {
				printf("%-10s\t\t\t%10.3lf\t\t%10.3lf\n", "", xArray[i], yArray[i]);
			}
			else if (number > (stdDevArrayY * 2)) {
				
			}
		}
	}
	else {
		printf("%-19s\t%18s\n", "Outliers(3x)", "no outliers");
	}
}

