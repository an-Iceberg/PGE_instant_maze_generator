#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <stack>

enum Direction
{
  NOT_SET,
  UP,
  LEFT,
  DOWN,
  RIGHT
};

class MazeGenerator : public olc::PixelGameEngine
{
public:
  MazeGenerator()
  {
    sAppName = "Instant Maze Generator";
  }

private:
  const int mazeWidth = 50; // Maze width in maze cells
  const int mazeHeight = 50; // Maze height in maze cells
  const int cellCount = 2500; // Number of cells in the maze
  const int pathWidth = 3; // Path width in pixels
  Direction maze [2500]; // Vector containing all cells and their data/information
  // TODO: maybe we can do without visitedCellsCounter
  int visitedCellsCounter; // Number of cells that has been visited
  std::stack<olc::vi2d> unvisitedCells; // Contains all maze cells (as coordinates) who's direction has not yet been set

public:
  bool OnUserCreate() override
  {
    // Initializing the random number generator
    srand(time(nullptr));

    visitedCellsCounter = cellCount + 1;

    // Set all the maze cells to have no direction
    for (int i = 0; i < cellCount; i++)
    {
      maze[i] = Direction{NOT_SET};
    }

    Clear(olc::BLACK);

    PaintingRoutine();

    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override
  {
    // Generate new maze when ENTER key is pressed
    if (GetKey(olc::Key::ENTER).bPressed)
    {
      Clear(olc::BLACK);

      visitedCellsCounter = 1;

      // Resetting all maze data
      for (Direction& cell : maze)
      {
        cell = Direction{NOT_SET};
      }

      // The top leftmost cell is going to be the starting point for the maze
      unvisitedCells.push(olc::vi2d{0, 0});

      // Create a new maze
      GenerateMaze();

      PaintingRoutine();
    }

    return true;
  }


  // ----------------------------------------------------------------------------------------------------


private:
  void GenerateMaze()
  {
    // As long as there are unvisited cells, update the maze
    while (visitedCellsCounter < cellCount)
    {
      std::vector<Direction> validNeighbours;

      // Checks if neighbours exist and if their direction has been set
      addAllValidNeighbours(validNeighbours);

      // If there are any valid neighbours choose a random one
      if (!validNeighbours.empty())
      {
        // Chooses a random neighbour from all valid neighbours
        Direction nextCellDirection = validNeighbours[rand() % validNeighbours.size()];

        Direction& currentCell = maze[IndexOfCurrentCell()];

        // Set the current cell's direction to point towards the selected neighbour
        currentCell = nextCellDirection;

        // Push the selected cell onto the stack
        unvisitedCells.push(CoordinatesOfNeighbour(nextCellDirection));

        visitedCellsCounter++;
      }
      // There are no valid neighbours so we need to back-track until we find some valid ones
      else
      {
        // Setting the enpoint cell's direction to point to its previous cell on the stack
        // While backtracking we reverse all the directions that have been set (dunno why but it seems to work)
        Direction& previousCell = maze[IndexOfCurrentCell()];

        unvisitedCells.pop();

        switch (maze[IndexOfCurrentCell()])
        {
          case UP:
            previousCell = DOWN;
          break;

          case LEFT:
            previousCell = RIGHT;
          break;

          case DOWN:
            previousCell = UP;
          break;

          case RIGHT:
            previousCell = LEFT;
          break;
        }
      }
    }
  }

  // Draws the maze to the screen
  void PaintingRoutine()
  {
    // Draws each cell
    for (int currentCellIndex = 0; currentCellIndex < cellCount; currentCellIndex++)
    {
      // int currentCellIndex = std::distance(maze.begin(), cell);

      // Calculating the x and y coordinates of the current cell
      // x = index % height
      // y = index / height
      olc::vi2d currentCell = {currentCellIndex % mazeHeight, currentCellIndex / mazeHeight};

      olc::Pixel interiorColor;

      // Paints the cell interior
      if (maze[currentCellIndex] == NOT_SET)
      {
        interiorColor = olc::BLUE;
      }
      else
      {
        interiorColor = olc::WHITE;
      }

      paintCellInterior(currentCell, interiorColor);
      paintCellWall(currentCell, maze[currentCellIndex]);

      // HACK: the -1 correction on the x is necessary for some reason
      currentCell.x--;

      // If this cell is the top of the stack
      if (!unvisitedCells.empty() && currentCell.x == unvisitedCells.top().x && currentCell.y == unvisitedCells.top().y)
      {
        // On the last painting cycle the top of the stack is painted as a regular cell
        if (visitedCellsCounter == cellCount)
        {
          paintCellInterior(currentCell, olc::WHITE);

          continue;
        }

        paintCellInterior(currentCell, olc::GREEN);
      }
    }
  }

  // Paints the interior of a cell
  void paintCellInterior(const olc::vi2d& currentCell, const olc::Pixel& interiorColor)
  {
    // Bottom triangle
    Draw(currentCell.x + (currentCell.x * pathWidth + 0) + 1, currentCell.y + (currentCell.y * pathWidth + 1) + 1, interiorColor);
    Draw(currentCell.x + (currentCell.x * pathWidth + 0) + 1, currentCell.y + (currentCell.y * pathWidth + 2) + 1, interiorColor);
    Draw(currentCell.x + (currentCell.x * pathWidth + 1) + 1, currentCell.y + (currentCell.y * pathWidth + 2) + 1, interiorColor);

    // Top triangle
    Draw(currentCell.x + (currentCell.x * pathWidth + 1) + 1, currentCell.y + (currentCell.y * pathWidth + 0) + 1, interiorColor);
    Draw(currentCell.x + (currentCell.x * pathWidth + 2) + 1, currentCell.y + (currentCell.y * pathWidth + 0) + 1, interiorColor);
    Draw(currentCell.x + (currentCell.x * pathWidth + 2) + 1, currentCell.y + (currentCell.y * pathWidth + 1) + 1, interiorColor);

    // Paints the cell diagonal
    for (int i = 0; i < pathWidth; i++)
    {
      Draw(currentCell.x + (currentCell.x * pathWidth + i) + 1, currentCell.y + (currentCell.y * pathWidth + i) + 1, interiorColor);
    }
  }

  // Paints the walls of a cell
  void paintCellWall(const olc::vi2d& currentCell, const Direction& direction)
  {
    for (int i = 0; i < (direction == NOT_SET ? pathWidth + 1 : pathWidth); i++)
    {
      switch (direction)
      {
        case NOT_SET:
          Draw(currentCell.x + (currentCell.x * pathWidth + pathWidth) + 1, currentCell.y + (currentCell.y * pathWidth + i) + 1, olc::BLACK);
          Draw(currentCell.x + (currentCell.x * pathWidth + i) + 1, currentCell.y + (currentCell.y * pathWidth + pathWidth) + 1, olc::BLACK);
        break;

        case UP:
          Draw(currentCell.x + (currentCell.x * pathWidth + i) + 1, currentCell.y + (currentCell.y * pathWidth - 1) + 1);
        break;

        case LEFT:
          Draw(currentCell.x + (currentCell.x * pathWidth - 1) + 1, currentCell.y + (currentCell.y * pathWidth + i) + 1);
        break;

        case DOWN:
          Draw(currentCell.x + (currentCell.x * pathWidth + i) + 1, currentCell.y + (currentCell.y * pathWidth + pathWidth) + 1);
        break;

        case RIGHT:
          Draw(currentCell.x + (currentCell.x * pathWidth + pathWidth) + 1, currentCell.y + (currentCell.y * pathWidth + i) + 1);
        break;
      }
    }
  }

  // Returns a vector of valid directions to choose from.
  // Maze cells outside the edge of the maze are not added to the vector
  void addAllValidNeighbours(std::vector<Direction>& neighbours)
  {
    // If the upper neighbour exists and its direction is NOT_SET, add it as a valid neighbour
    if (unvisitedCells.top().y > 0 && maze[IndexOfNeighbour(Direction{UP})] == NOT_SET)
    {
      neighbours.push_back(Direction{UP});
    }

    // If the left neighbour exists and its direction is NOT_SET, add it as a valid neighbour
    if (unvisitedCells.top().x > 0 && maze[IndexOfNeighbour(Direction{LEFT})] == NOT_SET)
    {
      neighbours.push_back(Direction{LEFT});
    }

    // If the lower neighbour exists and its direction is NOT_SET, add it as a valid neighbour
    if (unvisitedCells.top().y < mazeWidth - 1 && maze[IndexOfNeighbour(Direction{DOWN})] == NOT_SET)
    {
      neighbours.push_back(Direction{DOWN});
    }

    // If the right neighbour exists and its direction is NOT_SET, add it as a valid neighbour
    if (unvisitedCells.top().x < mazeHeight - 1 && maze[IndexOfNeighbour(Direction{RIGHT})] == NOT_SET)
    {
      neighbours.push_back(Direction{RIGHT});
    }
  }

  // Returns the index of a cell's neighbour in maze or the current cell's
  int IndexOfNeighbour(Direction direction)
  {
    int index;

    switch (direction)
    {
      case NOT_SET:
        index = (unvisitedCells.top().y) * mazeWidth + (unvisitedCells.top().x);
      break;

      case UP:
        index = (unvisitedCells.top().y - 1) * mazeWidth + (unvisitedCells.top().x);
      break;

      case LEFT:
        index = (unvisitedCells.top().y) * mazeWidth + (unvisitedCells.top().x - 1);
      break;

      case DOWN:
        index = (unvisitedCells.top().y + 1) * mazeWidth + (unvisitedCells.top().x);
      break;

      case RIGHT:
        index = (unvisitedCells.top().y) * mazeWidth + (unvisitedCells.top().x + 1);
      break;
    }

    return index;
  }

  // Returns the index of the element on the top of the stack
  int IndexOfCurrentCell()
  {
    return unvisitedCells.top().y * mazeWidth + unvisitedCells.top().x;
  }

  // Returns the coordinates of a cell neighbouring the current cell (top of the stack)
  olc::vi2d CoordinatesOfNeighbour(Direction direction)
  {
    olc::vi2d neighbour;

    switch (direction)
    {
      case NOT_SET:
        neighbour.x = unvisitedCells.top().x;
        neighbour.y = unvisitedCells.top().y;
      break;

      case UP:
        neighbour.x = unvisitedCells.top().x;
        neighbour.y = unvisitedCells.top().y - 1;
      break;

      case LEFT:
        neighbour.x = unvisitedCells.top().x - 1;
        neighbour.y = unvisitedCells.top().y;
      break;

      case DOWN:
        neighbour.x = unvisitedCells.top().x;
        neighbour.y = unvisitedCells.top().y + 1;
      break;

      case RIGHT:
        neighbour.x = unvisitedCells.top().x + 1;
        neighbour.y = unvisitedCells.top().y;
      break;
    }

    return neighbour;
  }
};

int main()
{
  MazeGenerator instance;

  if (instance.Construct(201, 201, 4, 4, false, true))
  {
    instance.Start();
  }

  return 0;
}
