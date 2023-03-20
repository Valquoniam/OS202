#include <SFML/Window/Keyboard.hpp>
#include <ios>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <tuple>
#include <chrono>
#include "cartesian_grid_of_speed.hpp"
#include "vortex.hpp"
#include "cloud_of_points.hpp"
#include "runge_kutta.hpp"
#include "screen.hpp"
#include "point.hpp"
#include <mpi.h>

auto readConfigFile( std::ifstream& input )
{
    using point=Simulation::Vortices::point;

    int isMobile;
    std::size_t nbVortices;
    Numeric::CartesianGridOfSpeed cartesianGrid;
    Geometry::CloudOfPoints cloudOfPoints;
    constexpr std::size_t maxBuffer = 8192;
    char buffer[maxBuffer];
    std::string sbuffer;
    std::stringstream ibuffer;
    // Lit la première ligne de commentaire :
    input.getline(buffer, maxBuffer); // Relit un commentaire
    input.getline(buffer, maxBuffer);// Lecture de la grille cartésienne
    sbuffer = std::string(buffer,maxBuffer);
    ibuffer = std::stringstream(sbuffer);
    double xleft, ybot, h;
    std::size_t nx, ny;
    ibuffer >> xleft >> ybot >> nx >> ny >> h;
    cartesianGrid = Numeric::CartesianGridOfSpeed({nx,ny}, point{xleft,ybot}, h);
    input.getline(buffer, maxBuffer); // Relit un commentaire
    input.getline(buffer, maxBuffer); // Lit mode de génération des particules
    sbuffer = std::string(buffer,maxBuffer);
    ibuffer = std::stringstream(sbuffer);
    int modeGeneration;
    ibuffer >> modeGeneration;
    if (modeGeneration == 0) // Génération sur toute la grille 
    {
        std::size_t nbPoints;
        ibuffer >> nbPoints;
        cloudOfPoints = Geometry::generatePointsIn(nbPoints, {cartesianGrid.getLeftBottomVertex(), cartesianGrid.getRightTopVertex()});
    }
    else 
    {
        std::size_t nbPoints;
        double xl, xr, yb, yt;
        ibuffer >> xl >> yb >> xr >> yt >> nbPoints;
        cloudOfPoints = Geometry::generatePointsIn(nbPoints, {point{xl,yb}, point{xr,yt}});
    }
    // Lit le nombre de vortex :
    input.getline(buffer, maxBuffer); // Relit un commentaire
    input.getline(buffer, maxBuffer); // Lit le nombre de vortex
    sbuffer = std::string(buffer, maxBuffer);
    ibuffer = std::stringstream(sbuffer);
    try {
        ibuffer >> nbVortices;        
    } catch(std::ios_base::failure& err)
    {
        std::cout << "Error " << err.what() << " found" << std::endl;
        std::cout << "Read line : " << sbuffer << std::endl;
        throw err;
    }
    Simulation::Vortices vortices(nbVortices, {cartesianGrid.getLeftBottomVertex(),
                                               cartesianGrid.getRightTopVertex()});
    input.getline(buffer, maxBuffer);// Relit un commentaire
    for (std::size_t iVortex=0; iVortex<nbVortices; ++iVortex)
    {
        input.getline(buffer, maxBuffer);
        double x,y,force;
        std::string sbuffer(buffer, maxBuffer);
        std::stringstream ibuffer(sbuffer);
        ibuffer >> x >> y >> force;
        vortices.setVortex(iVortex, point{x,y}, force);
    }
    input.getline(buffer, maxBuffer);// Relit un commentaire
    input.getline(buffer, maxBuffer);// Lit le mode de déplacement des vortex
    sbuffer = std::string(buffer,maxBuffer);
    ibuffer = std::stringstream(sbuffer);
    ibuffer >> isMobile;
    return std::make_tuple(vortices, isMobile, cartesianGrid, cloudOfPoints);
}


// Ce main sera parallelisé avec MPI. On aura
//      - Un processus dédié à l'interface graphique et utilisateur : le 0.
//      - Le reste des processus dédiés au calcul
int main( int nargs, char* argv[] )
{   
    /********** Initialisation MPI ************/
    int rank,size ;
    MPI_Init(&nargs, &argv);
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Comm_rank(comm, &rank);  //MPI compte les processus, etc...
    MPI_Comm_size(comm, &size); 
    MPI_Status status;
    /*******************************************/
    char const* filename;
    int touche;
    // Si on appelle mal le .exe
    if (nargs==1){
        
        //Le processus maître dit "erreur !"
        if (rank == 0) {
            std::cerr << "Usage: " << argv[0] << " <nom fichier configuration>" << std::endl;
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }
    // Ouverture et lecture du .dat
    filename = argv[1];
    std::ifstream fich(filename);
    auto config = readConfigFile(fich);
    fich.close();

    // Récupération des constantes pour tous les processus
    // En effet, le temps de lecture est faible. Il est plus efficace de lire n fois que de lire 1 fois et broadcast à n-1 processus.
    auto vortices = std::get<0>(config); // Position des vortex
    auto isMobile = std::get<1>(config); // Vortex stationnaire (0) ou mobiles (1)
    auto grid     = std::get<2>(config); // Le type de grille et coordonées
    auto cloud    = std::get<3>(config); // Génération des particules

    // Modification de la résolution de la fenêtre (de base, elle est en 800x600)
    std::size_t resx=800, resy=600;
    if (nargs>3)
    {
        resx = std::stoull(argv[2]);
        resy = std::stoull(argv[3]);
    }   

    grid.updateVelocityField(vortices);

    bool animate = false;
    double dt = 0.1;
    bool advance = false;

    class Geometry::CloudOfPoints subcloud;

/*********************************** PROCESSUS MAITRE/FENETRE ********************************/
    if (rank ==0){
        
        // Ecriture des instructions possibles
        std::cout << "######## Vortex simulator ########" << std::endl << std::endl;
        std::cout << "Press P for play animation " << std::endl;
        std::cout << "Press S to stop animation" << std::endl;
        std::cout << "Press right cursor to advance step by step in time" << std::endl;
        std::cout << "Press down cursor to halve the time step" << std::endl;
        std::cout << "Press up cursor to double the time step" << std::endl;

        // Initialisation : création de la fenêtre, sans animation avec un pas de tps de 0.1
        Graphisme::Screen myScreen( {resx,resy}, {grid.getLeftBottomVertex(), grid.getRightTopVertex()});

        // Boucle principale
        while (myScreen.isOpen())
        {   
            auto start = std::chrono::system_clock::now();
            advance = false;

            // On inspecte tous les évènements de la fenêtre qui ont été émis depuis la précédente itération
            // On note les évènements et on les broadcast aux processus qui s'occupent du calcul
            sf::Event event;
            while (myScreen.pollEvent(event))
            {   
                if (event.type == sf::Event::Closed){
                    touche = 0;
                    MPI_Bcast(&touche,sizeof(int),MPI_CHAR,0,comm);
                    myScreen.close();
                    return 0;
                    break;
                } else if (event.type == sf::Event::Resized){
                    myScreen.resize(event);
                } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)){
                    std::cout << "touche p préssée";
                    touche = 1;
                    MPI_Send(&touche,sizeof(int),MPI_INT,1 ,1,comm);
                    animate = true;
                } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)){
                    touche = 2;
                    MPI_Bcast(&touche,sizeof(int),MPI_INT,0,comm);
                    animate = false;
                } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)){
                    touche = 3;
                    MPI_Bcast(&touche,sizeof(int),MPI_INT,0,comm);
                    dt *= 2;
                } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)){
                    touche = 4;
                    MPI_Bcast(&touche,sizeof(int),MPI_INT,0,comm);
                    dt /= 2;
                } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)){
                    touche = 5;
                    MPI_Bcast(&touche,sizeof(int),MPI_INT,0,comm);
                    advance = true;}
            }

            if (animate | advance) {
                if (isMobile) {
                    vortices.recv(1, comm, &status);
                    grid.recv(1, comm, &status);
                }
                cloud.recv(1, comm, &status);
            }
            //Fond noir
            myScreen.clear(sf::Color::Black);

            //Affichage du pas de temps
            std::string strDt = std::string("Time step : ") + std::to_string(dt);
            myScreen.drawText(strDt, Geometry::Point<double>{50, double(myScreen.getGeometry().second-96)});

            //Affichage de la simulation
            myScreen.displayVelocityField(grid, vortices);
            myScreen.displayParticles(grid, vortices, cloud);

            // Calcul et affichage des FPS
            auto end = std::chrono::system_clock::now();
            std::chrono::duration<double> diff = end - start;
            std::string str_fps = std::string("FPS : ") + std::to_string(1./diff.count());
            myScreen.drawText(str_fps, Geometry::Point<double>{300, double(myScreen.getGeometry().second-96)});

            // Affichage de la fenêtre
            myScreen.display();
        }  

    }

/************************************ PROCESSUS DE CALCUL ********************************/
    if (rank == 1){
        while (touche != 0){
            advance = false;
            MPI_Recv(&touche, 1, MPI_INT, 0, 'e', comm, &status);
            // Traitement des ordres de l'utilisateur envoyés depuis le process 0
            if (touche == 5 ) {
                advance = true;
            } else if (touche == 1) {
                animate = true;
            } else if (touche == 2) {
                animate = false;
            } else if (touche == 3) {
                dt *= 2;
            } else if (touche == 4) {
                dt /= 2;
            } else if (touche == 0) {
                break;
            }

            if (animate | advance) {
                // Essai infructueux pour utiliser plusieurs processeurs pour le calcul :
                // Nous avons perdu beaucoup de temps sur cet essai

             /*   // Diviser la grille en sous-grilles pour chaque processus
                const int num_proc_calcul = size -1 ;
                float subgrid_width = grid.cellGeometry().first; float subgrid_height = grid.cellGeometry().second;
                const int subgrid_size = subgrid_width / num_proc_calcul;
                const float start_col = rank * subgrid_size;
                auto subgrid = Numeric::CartesianGridOfSpeed({subgrid_size,subgrid_height},{start_col,0}, grid.getStep());
                auto nb_points = cloud.numberOfPoints()/num_proc_calcul;
                auto subcloud = Geometry::generatePointsIn(nb_points,{{start_col,0},{start_col + subgrid_size-1,subgrid_height}});
                
                if (isMobile) {
                    subcloud = Numeric::solve_RK4_movable_vortices(dt, subgrid, vortices, subcloud);
                    vortices.send(0, comm);
                    grid.send(0, comm);
                
                } else {
                    subcloud = Numeric::solve_RK4_fixed_vortices(dt, subgrid, subcloud);
                }            
            }*/
                if (isMobile) {
                    cloud = Numeric::solve_RK4_movable_vortices(dt, grid, vortices, cloud);
                    vortices.send(0, comm);
                    grid.send(0, comm);
                } 
                
                else {
                    cloud = Numeric::solve_RK4_fixed_vortices(dt, grid, cloud);
                }
                cloud.send(0, comm);
            }
        }
    }

   /*Suite de l'essai infructeux MPI_Gather( subcloud.m_setOfPoints.data(), 
                (sizeof(Geometry::Point<double>) / sizeof(double)) * subcloud.m_setOfPoints.size(),
                MPI_DOUBLE,
                cloud.m_setOfPoints.data(),
                (size-1)*(sizeof(Geometry::Point<double>) / sizeof(double)) * subcloud.m_setOfPoints.size(),
                MPI_DOUBLE,
                0,
                comm); */
                                                                        
    MPI_Barrier(comm);
    MPI_Finalize();

    return EXIT_SUCCESS;
}
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
 /*     Geometry::CloudOfPoints clouds;
        // Si on est en mode animate OU que l'on est en mode advance OU les 2
            if ((animate | advance))
            {   
                std::cout << "animate\n";
                
                std::cout << "size: " <<size << std::endl;
                
                std::cout << "rank : " << my_rank << "; subgrid_size : " << subgrid_size << std::endl;
                const float start_col = my_rank * subgrid_size;
                std::cout << "start_col: " << start_col << std::endl;

                // Calculer la partie de la grille de chaque processus
                Numeric::CartesianGridOfSpeed subgrid;
                subgrid = Numeric::CartesianGridOfSpeed({subgrid_size,grid.m_height},{start_col,0}, grid.m_step);
                std::cout << "height: " << grid.m_height << std::endl;
                Geometry::CloudOfPoints subcloud;
                
                if (isMobile)
                {
                    auto subcloud = Numeric::solve_RK4_movable_vortices(dt, subgrid, vortices, cloud);
                    //cloud = Numeric::solve_RK4_movable_vortices(dt, grid, vortices, cloud);
                }
                else
                {
                    subcloud = Numeric::solve_RK4_fixed_vortices(dt, subgrid, cloud);
                    //cloud = Numeric::solve_RK4_fixed_vortices(dt, grid, cloud);
                }
                // Rassembler les résultats de chaque processus
                std::cout << "taille du ss_nuage : " << subcloud.numberOfPoints() << std::endl;
                MPI_Gather(&subcloud, subcloud.numberOfPoints() * sizeof(double), MPI_DOUBLE, &clouds, subcloud.numberOfPoints() * sizeof(double), MPI_DOUBLE, 0, MPI_COMM_WORLD);
            }

        // Code d'affichage par le processus 0:
        if (rank == 0){

            

        // synchronize MPI processes
        MPI_Barrier(MPI_COMM_WORLD);  
    }
    MPI_Finalize();
    return EXIT_SUCCESS;
 }
 */