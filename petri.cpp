#ifndef PETRI_DISH_CPP
  #define PETRI_DISH_CPP

  #include <math.h>
  #include <functional>
  #include <limits>

  struct Cell {
    float a, b;

    // Constructors
    Cell() : a(0), b(0) {}
    Cell(float conc) : a(conc), b(conc) {}
    Cell(float _a, float _b) : a(_a),b(_b) {}

    // Default copy, destructor, and assignment operator is fine
    //Cell& operator=(const Cell& c) { a = c.a; b = c.b; return *this; }
    //Cell(const Cell& c)  { *this = c; }

    // Arithmetic operators for notational bliss
    Cell  operator+  (const Cell& c) const { return Cell( a + c.a, b + c.b); }
    Cell  operator-  (const Cell& c) const { return Cell( a - c.a, b - c.b); }
    Cell  operator*  (const Cell& c) const { return Cell( a * c.a, b * c.b); } // For term by term multiplication
    Cell  operator*  (      float s) const { return Cell( a * s  , b * s   ); }
    friend Cell operator* (float s, const Cell& c) { return c * s; }
    Cell operator/ (float d) const {
      if (d == 0)
        return Cell(std::numeric_limits<float>::quiet_NaN());
      return Cell( a / d, b / d);
    }

    // Assignment operators
    Cell& operator+= (const Cell& c) { return *this = *this + c; }
    Cell& operator-= (const Cell& c) { return *this = *this - c; }
    Cell& operator*= (      float s) { return *this = *this * s; }
    Cell& operator/= (      float d) { return *this = *this / d; }

    // Array operator invokable on const Cell
    float operator[](int i) const {
      if (i < 0)
        i = i % 2 + 2;
      else
        i = i % 2;
      return (i == 0) ? a : b;
    }
    // Array operator invokable on Cell for editing components
    float& operator[](int i) {
      if (i < 0)
        i = i % 2 + 2;
      else
        i = i % 2;
      return (i == 0) ? a : b;
    }
  };

  struct updateStructure {
    Cell diffusion_rates;
    std::function<Cell(const Cell&)> feedRates;
    std::function<Cell(const Cell&)> reaction;
  };

  class PetriDish {
    Cell* dish;
    Cell* next_dish;
  
   public :
    const int width, height;

    PetriDish(int w, int h) : width(w), height(h), dish(nullptr), next_dish(nullptr) {
      do {
        dish = new Cell[w * h];
      } while (dish == nullptr);
      do {
        next_dish = new Cell[w * h];
      } while (next_dish == nullptr);
    }
    ~PetriDish() {
      delete[] dish;
      delete[] next_dish;
    }

    void swapDishes() {
      Cell* temp = dish;
      dish = next_dish;
      next_dish = temp;
    }

    Cell& getCellAt(int x, int y, bool access_from_current_dish = true) {
      x = (x %  width +  width) %  width;
      y = (y % height + height) % height;
      return (access_from_current_dish ? dish : next_dish)[x * height + y];
    }
    Cell& setCellAt(int x, int y, Cell& new_cell_value, bool access_from_current_dish = true) {
      x = (x %  width +  width) %  width;
      y = (y % height + height) % height;
      (access_from_current_dish ? dish : next_dish)[x * height + y] = new_cell_value;
      return (access_from_current_dish ? dish : next_dish)[x * height + y];
    }

    Cell laplaceAt(int x, int y) {
      Cell diff = Cell(), current_cell = getCellAt(x, y);
      for (int dx = -1; dx < 2; ++dx) {
        for (int dy = -1; dy < 2; ++dy) {
          float multiplier = 0;
          switch (abs(dx) + abs(dy)) {
            case 0 : multiplier = 0; break;
            case 1 : multiplier = 0.2; break;
            case 2 : multiplier = 0.05; break;
          }
          diff += (getCellAt(x + dx, y + dy) - current_cell) * multiplier;
        }
      }

      return diff;
    }

    void updateDish(float timestep, const updateStructure& reaction_data) {
      for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
          const Cell& current_cell = getCellAt(x, y, true);
          Cell& new_cell = getCellAt(x, y, false);
          Cell  del_conc = Cell();


          // dA/dt = D_A ∇²A + R_feed + R_reaction

          // Diffusion step
          del_conc += laplaceAt(x,y) * reaction_data.diffusion_rates;
          // Reaction step
          del_conc += reaction_data.reaction(current_cell);
          // Feed step
          del_conc += reaction_data.feedRates(current_cell);

          // Now update new dish
          new_cell = current_cell + del_conc * timestep;
        }
      }
      // Swap dishes once updated
      swapDishes();
    }

    void pushToPixels(uint8_t* pixel_array) {
      //static uint8_t c = 0;
      for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
          int pixel_index = 4 * (x * height + y);
          const Cell& current_cell = getCellAt(x, y, true);

          // R G B A
          pixel_array[pixel_index + 0] = 0x7F * ((current_cell.a ) + 1);
          pixel_array[pixel_index + 1] = 0xFF * 0;
          pixel_array[pixel_index + 2] = 0x7F * ((current_cell.b ) + 1);
          pixel_array[pixel_index + 3] = 0xFF ;
          // Test
          // pixel_array[pixel_index + 0] = c & 0xFF;
          // pixel_array[pixel_index + 1] = (y > 200) ? c*c : 0;
          // pixel_array[pixel_index + 2] = (x * x - 200 * y > 0) ? c*c*c : 0;
          // pixel_array[pixel_index + 3] = 255;
        }
      }
      //c++;
    }
  };

#endif