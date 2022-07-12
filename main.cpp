#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <cassert>
#include <time.h>
#include <chrono>
#include <thread>

#define OLC_PGE_APPLICATION
#include <olcPixelGameEngine.h>

enum DIR : uint8_t{
    UP, RIGHT, DOWN, LEFT
};

inline DIR ctodir(char c){
    switch(c){
    case 'U': return UP;
    case 'R': return RIGHT;
    case 'D': return DOWN;
    case 'L': return LEFT;
    }
    std::string s = "ERROR: got unkown character '" + std::string(1, c) + "' to `ctodir`. Returning DIR::UP\n";
    std::cout << s;
    throw std::out_of_range(s);
    return UP; 
}

template <typename T>
inline std::vector<T> uniun(std::vector<T> A, std::vector<T> B){
    std::vector<T> out;
    for (T& a: A){
        if (std::find(B.begin(), B.end(), a) != B.end()) out.push_back(a);
    }
    return out;
}

class Cell{
    size_t m_id;
    std::vector<size_t> vConnect[4];
    std::shared_ptr<olc::Decal> m_decal;
public:
    Cell(size_t id, const std::string sprite_path): m_id(id){
        for (int i=0; i<4; i++) vConnect[i] = std::vector<size_t>();
        m_decal = std::make_shared<olc::Decal>(new olc::Sprite(sprite_path));
    }
    Cell(const Cell& cell){
        *this = cell;
    }
    inline std::vector<size_t> getConnect(DIR d) const{
        return vConnect[d];
    }

    inline bool canConnect(size_t id, DIR dir){
        return std::find(vConnect[dir].begin(), vConnect[dir].end(), id) != vConnect[dir].end();
    }
    void setConnect(size_t id, DIR dir){
        if (!canConnect(id, dir))
            vConnect[dir].push_back(id);
    }
    inline std::shared_ptr<olc::Decal> decal() const{
        return m_decal;
    }
    Cell& operator = (const Cell& rhs){
        m_id = rhs.m_id;
         
        for (int i=0; i<4; i++) vConnect[i] = rhs.vConnect[i];
        m_decal = rhs.m_decal;
        return *this;
    }
};

struct GridCell{
    olc::vi2d pos;
    std::vector<size_t> vCanBe;
    
    void remove(size_t id){
        auto it = std::find(vCanBe.begin(), vCanBe.end(), id);
        if (it == vCanBe.end()) return;
        vCanBe.erase(it);
        assert(vCanBe.size() > 0);
    }
    inline size_t getRand(){
        return vCanBe[rand() % vCanBe.size()];
    }
};

class WaveFuncCollapse: public olc::PixelGameEngine
{
public:
	WaveFuncCollapse()
	{
		sAppName = "WaveFuncCollapase";
	}
    ~WaveFuncCollapse(){
        for (int y=0; y<grid.y; y++){
            delete[] vGrid[y];
            vGrid[y] = nullptr;
        }
        delete[] vGrid;
        vGrid = nullptr;
    }
    
    std::vector<Cell> vCells;
    olc::vi2d grid, decal;
    GridCell** vGrid;
    float scaler;

    float fFrameCounter;
    // every fFrameNext seconds the next iteration will be shown
    float fFrameNext = 0.01f;
    size_t nFrameNr = 0; // TODO: remove this variable
    
    bool bFinished = false;
    const float fMaxFPS = 30.0f;

    // TODO: remove this function
    void printGrid(){
         for (int y=0; y<grid.y; y++){
            for (int x=0; x<grid.x; x++){
                std::cout << vGrid[y][x].vCanBe.size() << " ";

            }
            std::cout << '\n';
        }
        std::cout << std::flush;
    }

    // TODO: create rule set from picture
public:
	bool OnUserCreate() override
	{
        // TODO
        srand(0);
        fFrameCounter = 0.0f;


        auto dirIter = std::filesystem::directory_iterator("./tiles");
        std::vector<std::string> vFileNames;
        int nFile = 0;

        for (auto& entry : dirIter){
            std::string s = entry.path().string();
            if (entry.is_regular_file() && s.substr(s.size() - 4, 4) == ".png"){
                 vFileNames.push_back(entry.path());
            }
        }
        std::sort(vFileNames.begin(),vFileNames.end(),[](auto& a, auto& b){ return a<b; });

        for (auto& sFileName: vFileNames){
            vCells.emplace_back(nFile++, sFileName);
        }
        std::ifstream file("./tiles/rules.txt");
        if (!file.is_open()){
            std::cerr << "Couldn't open tiles/rules.txt\n";
            return false;
        }
        std::string line;
        int nLine = 0;
        while (std::getline(file, line))
        {
            if (line.substr(0, 2) == "//" || line.empty()) {
                continue;
            }
            std::istringstream iss(line);
            if (nLine == 0){
                iss >> grid.x >> grid.y;
                this->vGrid = new GridCell*[grid.y];
                
                for (int y=0; y<grid.y; y++){
                    this->vGrid[y] = new GridCell[grid.x];
                    for (int x=0; x<grid.x; x++){
                        this->vGrid[y][x].pos = olc::vi2d{x,y};
                        for (size_t i=0; i<vCells.size(); i++) this->vGrid[y][x].vCanBe.push_back(i);
                    }
                }
            }else if (line[0] == 's'){ // set command
                std::string s;
                iss >> s;
                if (s == "sr"){ // set row
                    int y;
                    size_t val;
                    iss >> y >> val;
                    for (int x=0; x<grid.x; x++){
                        vGrid[y][x].vCanBe.clear();
                        vGrid[y][x].vCanBe.push_back(val);
                    }
                }else if (s == "sc"){ // set col

                    int x;
                    size_t val;
                    iss >> x >> val;
                    for (int y=0; y<grid.y; y++){
                        vGrid[y][x].vCanBe.clear();
                        vGrid[y][x].vCanBe.push_back(val);
                    }
                }else { // set cell
                    int x, y;
                    size_t val;

                    iss >> x >> y >> val;
                    vGrid[y][x].vCanBe.clear();
                    vGrid[y][x].vCanBe.push_back(val);
                }
            }else{
                int cell;
                char d;
                iss >> cell >> d;
                while (true){
                    size_t canBe;
                    if (!(iss >> canBe)) break;
                    vCells[cell].setConnect(canBe, ctodir(d));

                    vCells[canBe].setConnect(cell, (DIR)(((int)ctodir(d) + 2) % 4));
                }
                
            }
            nLine++;
        }
        file.close();
       
        decal.x = vCells[0].decal()->sprite->width;
        decal.y = vCells[0].decal()->sprite->height;

        float scaler_x = (float)ScreenWidth()  / float(decal.x * grid.x);
        float scaler_y = (float)ScreenHeight() / float(decal.y * grid.y);
        scaler = (scaler_x < scaler_y)? scaler_x : scaler_y;

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
        auto start = std::chrono::steady_clock::now();
        
        fFrameCounter += fElapsedTime;
        if (GetKey(olc::ESCAPE).bPressed) return false;
        


        if (GetKey(olc::SHIFT).bPressed)  fFrameNext /= 4.0f;
        if (GetKey(olc::SHIFT).bReleased) fFrameNext *= 4.0f;

        // draw screen
        Clear(olc::BLACK);
        for (int y=0; y<grid.y; y++){
            for (int x=0; x<grid.x; x++){
                assert(vGrid[y][x].vCanBe.size() > 0);
                if (vGrid[y][x].vCanBe.size() == 1){
                    size_t i = vGrid[y][x].vCanBe[0];
                    olc::vf2d pos{float(x*decal.x), float(y*decal.y)};
                    DrawDecal(pos * scaler, vCells[i].decal().get(), olc::vf2d{scaler, scaler});
                    // DrawRect(x * sprite.x * scaler, y * sprite.y * scaler, sprite.x * scaler, sprite.y * scaler);
                }
            }
        }

        if (bFinished){
            auto end = std::chrono::steady_clock::now();
            size_t t = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            if (t < size_t(1000.0f/fMaxFPS)){
                std::chrono::milliseconds timespan(size_t(1000.0f/fMaxFPS) - t);
                std::this_thread::sleep_for(timespan);
            }
            return true;
        }

        if (fFrameCounter < fFrameNext) return true;
        
        do{ fFrameCounter -= fFrameNext;
        }while(fFrameCounter >= fFrameNext);
        nFrameNr++;
        
        // get min cells
        std::vector<GridCell*> vMinCells;
        size_t nMinVal = SIZE_MAX;
        for (int y=0; y<grid.y; y++){
            for (int x=0; x<grid.x; x++){
                std::vector<size_t>& vCanBe = vGrid[y][x].vCanBe;
                if (vCanBe.size() == 1) continue;
                if (vCanBe.size() == nMinVal)
                    vMinCells.push_back(&vGrid[y][x]);
                else if (vCanBe.size() < nMinVal){
                    nMinVal = vCanBe.size();
                    vMinCells.clear();
                    vMinCells.push_back(&vGrid[y][x]);                    
                }
            }
        }
        
        // we got to the end
        if (vMinCells.size() == 0){
            bFinished = true;
            return true;
        }

        GridCell* randCell = vMinCells[rand() % vMinCells.size()];
        while (true){
            if (randCell->vCanBe.size() == 0){
                std::cout << "Broken Rules" << std::endl;
                bFinished = true;
                return true;
            }
            size_t id = randCell->getRand();

            Cell& cell = vCells[id];
            olc::vi2d& pos = randCell->pos;
            bool ok = true;
            // check up
            if (pos.y > 0){
                std::vector<size_t> vUnion = uniun(vGrid[pos.y-1][pos.x].vCanBe, cell.getConnect(UP));
                ok &= !vUnion.empty();
            }
            // check right
            if (pos.x+1 < grid.x){
                std::vector<size_t> vUnion = uniun(vGrid[pos.y][pos.x+1].vCanBe, cell.getConnect(RIGHT));
                ok &= !vUnion.empty();
            }
            // check down
            if (pos.y+1 < grid.y){
                std::vector<size_t> vUnion = uniun(vGrid[pos.y+1][pos.x].vCanBe, cell.getConnect(DOWN));
                ok &= !vUnion.empty();
            }
            // check left
            if (pos.x > 0){
                std::vector<size_t> vUnion = uniun(vGrid[pos.y][pos.x-1].vCanBe, cell.getConnect(LEFT));
                ok &= !vUnion.empty();
            }
            if (!ok){
                randCell->remove(id);
                if (randCell->vCanBe.size() == 1) return true;
                continue;
            }

            // update up
            if (pos.y > 0){
                std::vector<size_t> vUnion = uniun(vGrid[pos.y-1][pos.x].vCanBe, cell.getConnect(UP));
                vGrid[pos.y-1][pos.x].vCanBe = vUnion;
            }
            // update right
            if (pos.x+1 < grid.x){
                std::vector<size_t> vUnion = uniun(vGrid[pos.y][pos.x+1].vCanBe, cell.getConnect(RIGHT));
                vGrid[pos.y][pos.x+1].vCanBe = vUnion;
            }
            // update down
            if (pos.y+1 < grid.y){
                std::vector<size_t> vUnion = uniun(vGrid[pos.y+1][pos.x].vCanBe, cell.getConnect(DOWN));
                vGrid[pos.y+1][pos.x].vCanBe = vUnion;
            }
            // update left
            if (pos.x > 0){
                std::vector<size_t> vUnion = uniun(vGrid[pos.y][pos.x-1].vCanBe, cell.getConnect(LEFT));
                vGrid[pos.y][pos.x-1].vCanBe = vUnion;
            }
        
            randCell->vCanBe.clear();
            randCell->vCanBe.push_back(id);

            break;
        }
        
		return true;
	}
};


int main()
{
    WaveFuncCollapse demo;
	if (demo.Construct(240, 240, 4, 4))
		demo.Start();

	return 0;
}
