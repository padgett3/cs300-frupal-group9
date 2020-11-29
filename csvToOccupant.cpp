#include "csvToOccupant.h"

TileOccupant *newOccupant(const string &type, const string &csv)
{
        /* The right operand would convert to string implicitly
         * just fine but still doing explicit conversion.
         */
        if (type == string("Food"))
                return csvToFood(csv);
        else if (type == string("Binoculars"))
                return csvToBinoculars(csv);
        else if (type == string("Ship"))
                return csvToShip(csv);
        else if (type == string("Treasure"))
                return csvToTreasure(csv);
        else if (type == string("Clue"))
                return csvToClue(csv);
        else if (type == string("Diamond"))
                return csvToDiamond(csv);
        else if (type == string("Obstacle"))
                return csvToObstacle(csv);
        else if (type == string("Tool"))
                return csvToTool(csv);
        else
                return 0;
}

TileOccupant *csvToFood(const string &csv)
{
        stringstream input(csv);
        string name, temp;
        int price, energy; 
        
        getline(input, name, ',');
        getline(input, temp, ',');
        price = stoi(temp);
        getline(input, temp, ',');
        energy = stoi(temp);
        
        return new Food(name, price, energy);
}

TileOccupant *csvToBinoculars(const string &csv)
{
        stringstream input(csv);
        int price;
        
        input >> price;       
        
        return new Binoculars(price);
}

TileOccupant *csvToShip(const string &csv)
{
        stringstream input(csv);
        int price;
        
        input >> price;       
        
        return new Ship(price);
}

TileOccupant *csvToTreasure(const string &csv)
{
        stringstream input(csv);
        int worth;
        
        input >> worth;       
        
        return new Treasure(worth);
}

TileOccupant *csvToClue(const string &csv)
{
        return new Clue(csv);
}

TileOccupant *csvToDiamond(const string &csv)
{
        return new Diamond;
}

TileOccupant *csvToObstacle(const string &csv)
{
        stringstream input(csv);
        string name, temp;
        int energyCost;
        
        getline(input, name, ',');
        getline(input, temp, ',');
        energy = stoi(temp);
        
        return new Obstacle(name, energyCost);
}

TileOccupant *csvToTool(const string &csv)
{
        stringstream input(csv);
        string name, temp;
        int price, rating, vecLen;
                
        getline(input, name, ',');
        getline(input, temp, ',');
        price = stoi(temp);
        getline(input, temp, ',');
        rating = stoi(temp);
        getline(input, temp, ',');
        vecLen = stoi(temp);
        
        vector<string> usableOn(vecLen, "");
        
        for(int i = 0; i < vecLen; ++i)
        {
                getline(input, temp, ',');
                usableOn.at(i) = temp;
        }
        
        return new Tool(name, price, rating, usableOn);
}


