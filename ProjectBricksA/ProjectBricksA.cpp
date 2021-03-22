#include <iostream>
#include <fstream>
#include <string>

#include <opencv/cv.h>
#include <opencv2/imgproc.hpp>
#include <opencv/highgui.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <stdlib.h>
#include <stdio.h>

using namespace std;
using namespace cv;

#define blocks 48000

double how_many_other_pix(Mat img);	//подсчет пикселей
double how_many_blocks(double pix);	//подсчет блоков

void cutim(Mat img);				//ф-я "вырезания" кирпичей (объектов)
void feduge(Mat img, int fx);		//ф-я создания настраиваемой "маски" кирпичей
void inserte(Mat img);				//ф-я для предобработки изображения, в основном для определения координат
void delnois(Mat img);				//еще одна ф-я для предобработки изображения, удаляет" по 1-2 пикселя вокруг кирпича, которые в основном являются кривым срезом кирпича
void dotedraw(Mat img);				//ф-я рисования точки в центре кирпича для визуализации

int x, y = 0;

int main(int argc, char* argv[])
{
	int flag = 0;
	cin >> flag;

	Mat img = imread("DSC_0007.jpg");
	Mat img2;
	Mat img3;

	switch (flag) {
	case 3:
		cout << fixed;
		img2 = imread("DSC.png");
		cout << "Blocks: " << how_many_blocks(how_many_other_pix(img2)) << "\n";
	case 4:
		cout << fixed;
		img2 = imread("DSC.png");
		cout << "Pix: " << how_many_other_pix(img2) << "\n";
	case 5:
		feduge(img, 1);
		imwrite("DSC.png", img);
		img2 = imread("DSC.png");

		inserte(img2);
		imwrite("DSCinserte.png", img2);
		img3 = imread("DSCinserte.png");

		delnois(img3);
		imwrite("DSCdelnois.png", img3); //Это изображение необходимо передавать в программу для поиска координат
		break;
	case 6:
		cutim(img);						//Вырезание кирпичей по координатам
		break;
	case 7:
		dotedraw(img);					//Граф.представление центров кирпичей
		break;
	case 8:
		//Для обработки отдельного кирпича
		for (int i = 0; i < 44; i++) {
			string names_fo = "new/" + to_string(i + 1) + "_cutDSC.png";
			string names_fs = "newfx/cutDSCfx_" + to_string(i + 1) + ".png";

			img2 = imread(names_fo);
			feduge(img2, 0); //Для примера переключать второй аргумент с 0 на 1
			imwrite(names_fs, img2);
		}
		break;

	default:
		cout << "<empty, sorry>" << endl;
	}

	system("pause");
	return 0;
}

void cutim(Mat img)
{
	int road[4][2], //строка координат
		dote[4];

	int h, //hight
		w; //width

	int sl, //самая left
		su, //самая up
		sr, //самая right
		sd; //самая down

	int sX, //start X
		sY; //start Y

	string name_f;
	string aname;
	string num0 = "new/";
	string num1 = "_cutDSC.png";
	string num2 = "test/asd";

	ifstream fromFile;
	fromFile.open("SortCoord.txt", ios::in);
	if (!fromFile.is_open())
	{
		cout << "Don't open!\n";
	}
	else {
		int pil = 1;
		while (pil <= 44) //44 - потому что мы знаем, 11 кирпичей по вертикале х 4 кирпичей по горизонтале = 44
		{
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 2; j++) {
					fromFile >> road[i][j];
				}
			}

			sX = road[0][0];
			sr = 0;
			for (int i = 1; i < 4; i++)
				if (road[i][0] < sX) {
					sX = road[i][0];
					if (road[i - 1][0] > sr)
						sr = road[i - 1][0];
				}
				else {
					if (road[i][0] > sr)
						sr = road[i][0];
				}


			sY = road[0][1];
			sd = 0;
			for (int i = 1; i < 4; i++)
				if (road[i][1] < sY) {
					sY = road[i][1];
					if (road[i - 1][1] > sd)
						sd = road[i - 1][1];
				}
				else {
					if (road[i][1] > sd)
						sd = road[i][1];
				}


			h = abs(sY - sd); //высота изображения, разница самой верхней и самой нижней точки
			w = abs(sr - sX); //тоже самое для ширины

			cout << "[#: " << pil << ", h: " << h << ", w: " << w << "]" << endl;

			Mat ROI(img, Rect(sX, sY, w, h));
			Mat croppedImmage;
			ROI.copyTo(croppedImmage);

			name_f = num0 + to_string(pil) + num1;
			imwrite(name_f, croppedImmage);

			/*Rect r = Rect(sX, sY, w, h);
			cout << "Im draw in x: " << sX << " and y: " << sY << " rect: " << w << "x" << h << endl;
			rectangle(img, r, Scalar(255, 100, 100), 3, 8, 0);

							// Запускать это ТОЛЬКО тогда, когда нужно визуально посмотреть проходимость. Это необязательная часть программы, реализована условно на случай, если вдруг понадобится.
							// Если требуется визуализация, то запускать всю программу нужно два раза в следующей последовательности:
							//	1 сперва раскоментить 3 строки кода выше и 2 строки кода ниже
							//	2 затем запустить программу
							//	3 затем закоментить снова
							//	4 и снова запустить программу
							// ИНАЧЕ на вырезанных кирпичах будут лишние линии визуализации! (Потому что это по сути не нужно)
							// P.s. отрисовка визуальных областей для изображения 3000х2000 очень трудоемкая задача и, следовательно, долгая!

			aname = num2 + to_string(pil) + ".png";
			imwrite(aname, img);*/

			pil++;
			//system("pause");
		}
	}

	fromFile.close();
}

double how_many_other_pix(Mat img)
{
	int i, j, R, G, B, gor = 0;
	double c = 0.0;
	for (i = 0; i < img.cols; i++) {
		for (j = 0; j < img.rows; j++) {
			int k = img.at<uchar>(j, i);
			Vec3b intest = img.at<Vec3b>(j, i);
			R = intest[2];
			G = intest[1];
			B = intest[0];

			if (k != 0) {
				c++;
			}
			if (B == 250 && G == 20 && R == 20) {
				gor++;
			}
		}
	}

	cout << "Pix: " << c << "\n";
	cout << "Gor: " << gor << "\n";

	return c;
}

double how_many_blocks(double pix)
{
	long double h_blocks;
	h_blocks = pix / (blocks);
	//cout << "Blocks: " << h_blocks << "\n";

	long double dx = 1;

	if (h_blocks > 0) {
		h_blocks = floor((h_blocks * dx) + 0.5) / dx;
	}
	else {
		h_blocks = ceil((h_blocks * dx) - 0.5) / dx;
	}

	return h_blocks;
}

void feduge(Mat img, int fx)
{
	if (fx != 0 && fx != 1) {
		fx = 1;
	}
	int i, j = 0;
	double R, G, B = 0.0;
	double GR, BR = 0.0;


	for (i = 0; i < img.cols; i++)
	{
		for (j = 0; j < img.rows; j++)
		{
			Vec3b intest = img.at<Vec3b>(j, i);
			R = intest[2];
			G = intest[1];
			B = intest[0];

			GR = G / R;
			BR = B / R;

			if ((GR > 0.5) && (GR < 0.7) && (BR > 0.3) && (BR < 0.5) && (R > 150)) //кирпич
			{
				intest[0] = 55;
				intest[1] = 135;
				intest[2] = 200;

				img.at<Vec3b>(j, i) = intest;
			}
			else if ((GR > 0.6) && (GR < 0.75) && (BR > 0.40) && (BR < 0.65) && (R > 150)) //другой цвет
			{
				if (fx == 0)
				{
					intest[0] = 20;
					intest[1] = 20;
					intest[2] = 250;

				}
				else if (fx == 1) //если не нужно выделять другим цветом
				{
					intest[0] = 55;
					intest[1] = 135;
					intest[2] = 200;
				}

				img.at<Vec3b>(j, i) = intest;
			}
			else
			{
				intest[0] = 0;
				intest[1] = 0;
				intest[2] = 0;

				img.at<Vec3b>(j, i) = intest;
			}
		}
	}
}

void inserte(Mat img)
{
	int i, j = 1;
	int black = 0;
	int count = 0;
	cout << "Cols: " << img.cols << endl << "Rows: " << img.rows << endl << "CxR: " << img.cols * img.rows << endl;

	for (i = 1; i < img.cols - 1; i++)
	{
		for (j = 1; j < img.rows - 1; j++)
		{
			Vec3b intest = img.at<Vec3b>(j, i);

			Vec3b coord[4];
			coord[0] = img.at<Vec3b>(j, i - 1);
			coord[1] = img.at<Vec3b>(j, i + 1);
			coord[2] = img.at<Vec3b>(j - 1, i);
			coord[3] = img.at<Vec3b>(j + 1, i);

			if (intest[0] == 0 && intest[1] == 0 && intest[2] == 0)
			{
				black = 1;
			}
			else
			{
				black = 0;
			}
			/*
				intest[0] = 55;
				intest[1] = 135;
				intest[2] = 200;
			*/
			if ((coord[0][0] == 55 && coord[0][1] == 135 && coord[0][2] == 200) && (black == 1))
			{
				coord[0][0] = 160;
				coord[0][1] = 70;
				coord[0][2] = 160;
				//cout << "_UPUPUP_" << endl;

				img.at<Vec3b>(j, i - 1) = coord[0];
			}

			if ((coord[1][0] == 55 && coord[1][1] == 135 && coord[1][2] == 200) && (black == 1))
			{
				coord[1][0] = 160;
				coord[1][1] = 70;
				coord[1][2] = 160;
				//cout << "_DOWNDOWNDOWN_" << endl;

				img.at<Vec3b>(j, i + 1) = coord[1];
			}

			if ((coord[2][0] == 55 && coord[2][1] == 135 && coord[2][2] == 200) && (black == 1))
			{
				coord[2][0] = 160;
				coord[2][1] = 70;
				coord[2][2] = 160;
				//cout << "_LEFTLEFTLEFT_" << endl;

				img.at<Vec3b>(j - 1, i) = coord[2];
			}

			if ((coord[3][0] == 55 && coord[3][1] == 135 && coord[3][2] == 200) && (black == 1))
			{
				coord[3][0] = 160;
				coord[3][1] = 70;
				coord[3][2] = 160;
				//cout << "_RIGHTRIGHTRIGHT_" << endl;

				img.at<Vec3b>(j + 1, i) = coord[3];
			}

			black = 0;
			count++;
		}
	}

	cout << count << endl;
}

void delnois(Mat img)
{
	int i, j = 1;

	for (i = 0; i < img.cols; i++)
	{
		for (j = 0; j < img.rows; j++)
		{
			Vec3b intest = img.at<Vec3b>(j, i);

			if (intest[0] == 160 && intest[1] == 70 && intest[2] == 160)
			{
				intest[0] = 0;
				intest[1] = 0;
				intest[2] = 0;

				img.at<Vec3b>(j, i) = intest;
			}
		}
	}
}

void dotedraw(Mat img)
{
	int dote_coord[2];

	string name_f;
	string aname;
	string num0 = "new2/";
	string num1 = "dote/dote_";

	ifstream fromFile;
	fromFile.open("DoteCord.txt", ios::in);
	if (!fromFile.is_open())
	{
		cout << "Don't open!\n";
	}
	else {
		int pil = 1;
		while (pil <= 44)
		{			// ld -> lu -> ru -> rd
			fromFile >> dote_coord[0]; fromFile >> dote_coord[1];

			Rect r = Rect(dote_coord[0] - 10, dote_coord[1] - 10, 20, 20);
			cout << "Im draw in x: " << dote_coord[0] - 10 << " and y: " << dote_coord[0] - 10 << " rect: " << 20 << "x" << 20 << endl;
			rectangle(img, r, Scalar(255, 100, 100), 5, 8, 0);

			aname = num1 + to_string(pil) + ".png";
			imwrite(aname, img);

			pil++;
			//system("pause");
		}
	}

	fromFile.close();
}