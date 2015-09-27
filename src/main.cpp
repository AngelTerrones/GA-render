#include <cstdio>
#include <unistd.h>
#include <iostream>
#include <QDir>
#include <QtGui>
#include <QDebug>
#include <QSvgGenerator>
#include "Chromosome.hpp"

#define N_POLYGONS 50
#define N_POPULATION 10
#define HAREM 0.4

// global variables... Oh HELL
int max_generation = -1;
QImage sourceImage;
int imgW = -1;
int imgH = -1;
Chromosome *pobA[N_POPULATION];
Chromosome *pobB[N_POPULATION];
int sizePobA = -1;
int sizePobB = -1;

// functions
QImage DrawImage(Chromosome *dna){
    QImage image(imgW, imgH, QImage::Format_RGB32);
    QPainter painter(&image);
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setBrush(QBrush(QColor(0, 0, 0, 255)));
    QPolygon polyclean;
    int pointsclean[] = {0, 0, 0, imgH, imgW, imgH, imgW, 0};
    polyclean.setPoints(4, pointsclean);
    painter.drawConvexPolygon(polyclean);

    Polygon *poly = dna->DNA();
    for (int n = 0; n < N_POLYGONS; n++) {
        QColor color(poly[n].Red(),
                     poly[n].Green(),
                     poly[n].Blue(),
                     poly[n].Alpha());
        painter.setBrush(QBrush(color));

        int size = poly[n].NPoints();
        int *listPoints = new int[2*size];

        for(int i = 0; i < size; i++){
            listPoints[2*i] = poly[n].Vertex()[i].x();
            listPoints[2*i + 1] = poly[n].Vertex()[i].y();
        }

        QPolygon qPoly;
        qPoly.setPoints(size, listPoints);
        painter.drawConvexPolygon(qPoly);
        delete [] listPoints;
    }
    painter.end();

    return image;
}

void DrawSVG(Chromosome *dna, const char *svgName){
    QSvgGenerator svgGen;
    svgGen.setFileName(QString(svgName));
    svgGen.setSize(QSize(imgW, imgH));
    svgGen.setViewBox(QRect(0, 0, imgW, imgH));

    QPainter painter(&svgGen);
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setBrush(QBrush(QColor(0, 0, 0, 255)));
    QPolygon polyclean;
    int pointsclean[] = {0, 0, 0, imgH, imgW, imgH, imgW, 0};
    polyclean.setPoints(4, pointsclean);
    painter.drawConvexPolygon(polyclean);

    Polygon *poly = dna->DNA();
    for (int n = 0; n < N_POLYGONS; n++) {
        QColor color(poly[n].Red(),
                     poly[n].Green(),
                     poly[n].Blue(),
                     poly[n].Alpha());
        painter.setBrush(QBrush(color));

        int size = poly[n].NPoints();
        int *listPoints = new int[2*size];

        for(int i = 0; i < size; i++){
            listPoints[2*i] = poly[n].Vertex()[i].x();
            listPoints[2*i + 1] = poly[n].Vertex()[i].y();
        }

        QPolygon qPoly;
        qPoly.setPoints(size, listPoints);
        painter.drawConvexPolygon(qPoly);
        delete [] listPoints;
    }
    painter.end();
}

unsigned long long Distance(Chromosome *dna){
    int nBytesImage = sourceImage.byteCount();
    QImage imageRender = DrawImage(dna);
    unsigned long long distance = 0;
    const unsigned char *original = sourceImage.bits();
    const unsigned char *render = imageRender.bits();

    for (int i = 0; i < nBytesImage; i++) {
        int temp = original[i] - render[i];
        distance += temp*temp;
    }

    return distance;
}

void Crossover(Chromosome *a, Chromosome *b){
    int length = a->Length();

    if(length <= 0)
        return;

    // Chromosome cA, cB;
    // cA.Create(length);
    // cB.Create(length);

    for(int i = 0; i < length; i++){
        if(rand() % 2){
            Polygon tmp = a->DNA()[i];
            a->DNA()[i] = b->DNA()[i];
            b->DNA()[i] = tmp;
        }
    }

    // for(int i = 0; i < length; i++){
    //     if(rand() % 2){
    //         cA.DNA()[i] = a->DNA()[i];
    //         cB.DNA()[i] = b->DNA()[i];
    //     }else{
    //         cA.DNA()[i] = b->DNA()[i];
    //         cB.DNA()[i] = a->DNA()[i];
    //     }
    // }

    // for(int i = 0; i < length; i++){
    //     a->DNA()[i] = cA.DNA()[i];
    //     b->DNA()[i] = cB.DNA()[i];
    // }

    // cA.Delete();
    // cB.Delete();
}

Chromosome *SelectBest(){
    if(sizePobA == 0)
        return NULL;

    unsigned long long min = ~0x00;
    int index = 0;

    for(int i = 0; i < sizePobA; i++){
        if(pobA[i]->Fitness() < min){
            min = pobA[i]->Fitness();
            index = i;
        }
    }
    return pobA[index];
}

Chromosome *SelectTournament(){
    int i = rand() % sizePobA;
    int j = rand() % sizePobA;

    if(pobA[i]->Fitness() < pobA[j]->Fitness())
        return pobA[i];
    else
        return pobA[j];
}

void InsertPobB(Chromosome *dna){
    if(sizePobA <= sizePobB)
        return;

    if(pobB[sizePobB] == NULL){
        pobB[sizePobB] = new Chromosome();
    }

    dna->Clone(pobB[sizePobB]);

    sizePobB++;
}

void UpdatePopulation(){
    for(int i = 0; i < sizePobB; i++){
        pobA[i]->Delete();
        delete pobA[i];
        pobA[i] = new Chromosome();
        pobB[i]->Clone(pobA[i]);
    }

    sizePobA = sizePobB;
    sizePobB = 0;
}

void GAStep(){
    Chromosome *elite = SelectBest();
    Chromosome clonA;
    Chromosome clonB;
    Chromosome *ap = NULL;

    elite->Clone(&clonA);
    InsertPobB(&clonA);

    // harem
    int fraction = int(HAREM * sizePobA);
    for(int i = 0; i < fraction; i += 2){
        elite->Clone(&clonA);
        SelectTournament()->Clone(&clonB);
        Crossover(&clonA, &clonB);
        clonA.Mutate();
        clonB.Mutate();

        clonA.Fitness() = Distance(&clonA);
        clonB.Fitness() = Distance(&clonB);

        InsertPobB(&clonA);
        InsertPobB(&clonB);
    }

    while((sizePobA - sizePobB) > 0){
        ap = SelectTournament();
        ap->Clone(&clonA);
        ap = SelectTournament();
        ap->Clone(&clonB);

        Crossover(&clonA, &clonB);
        clonA.Mutate();
        clonB.Mutate();
        clonA.Fitness() = Distance(&clonA);

        clonB.Fitness() = Distance(&clonB);

        if(clonA.Fitness() < clonB.Fitness())
            InsertPobB(&clonA);
        else
            InsertPobB(&clonB);
    }
    UpdatePopulation();
}

void RunGA(){
    int generations = 0;
    while(max_generation >= 0 && generations <= max_generation){
        GAStep();
        if(generations % 100 == 0){
            std::cout << "Generacion: " << generations << "\n";
        }
        if(generations % 1000 == 0){
            Chromosome *best = SelectBest();
            QImage image = DrawImage(best);
            image.save(QString("out/out_") + QString::number(generations) + ".png");
            qDebug() << "Fitness: " << best->Fitness();
        }
        generations++;
    }
    DrawSVG(SelectBest(), "Final.svg");
}

void InitGA(){
    sizePobA = sizePobB = 0;

    for(int i = 0; i < N_POPULATION; i++){
        // population A
        pobA[i] = new Chromosome();
        pobA[i]->Create(N_POLYGONS);
        for(int j = 0; j < N_POLYGONS; j++){
            pobA[i]->DNA()[j].Init(imgW, imgH);
        }
        pobA[i]->Fitness() = Distance(pobA[i]);
        sizePobA++;

        // population B
        pobB[i] = NULL;
    }
}

void CleanUp(){
    for(int i = 0; i < sizePobA; i++){
        pobA[i]->Delete();
        delete pobA[i];
        pobB[i]->Delete();
        delete pobB[i];
    }
}

int main(int argc, char *argv[]){
    int option;

    while((option = getopt(argc, argv, "g:")) != -1){
        switch (option) {
            // get the generation limit
            case 'g': {
                max_generation = atoi(optarg);
                if(max_generation == 0){
                    std::cerr << "ERROR: Especificar limite de generarion mayor a cero: -g 100\n";
                    exit(EXIT_FAILURE);
                }
                else if(max_generation < 0){
                    std::cerr << "ERROR: El numero de generaciones debe ser positivo\n";
                    exit(EXIT_FAILURE);
                }
                break;
            }
            default:
                break;
        }
    }

    // get source file
    if(optind >= argc){
        std::cerr << "Especificar imagen a replicar\n";
        exit(EXIT_FAILURE);
    }

    // check if the file exists.
    FILE *fp = fopen(argv[optind], "r");
    if(fp == NULL){
        std::cerr << "ERROR: Imagen no existe: " << argv[optind] << "\n";
        exit(EXIT_FAILURE);
    }
    fclose(fp);

    // cargar imagen y hacer magia.
    sourceImage = QImage(argv[optind]);
    imgW = sourceImage.width();
    imgH = sourceImage.height();

    std::cout << "Maximo de generaciones: " << max_generation << "\n";
    QDir::current().mkpath("out");
    InitGA();
    RunGA();
    CleanUp();
    return 0;
}
