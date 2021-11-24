#include <SFML/Graphics.hpp>
#include "OpenSimplexNoise.cpp"
#include <iostream>
#include "petri.cpp"

#include <chrono>
#include <time.h>
  #include <random>
  using namespace std;
  using namespace chrono;
  
double random_float() {
  static mt19937 generator(chrono::steady_clock::now().time_since_epoch().count());
  static uniform_real_distribution<double> distribution(0.0, 1.0);
  return distribution(generator);
}

#define w 600
#define h 600
using namespace std;

struct PixelDisplayer{
  sf::Texture texture_from_pixels;
  sf::RectangleShape screen;

  PixelDisplayer(int width, int height) : screen(sf::Vector2f(width, height)) {
    texture_from_pixels.create(width, height);
    
    screen.setFillColor(sf::Color::White); // Only colors set here are rendered
    screen.setTexture(&texture_from_pixels);
    screen.setOrigin(0,0);
  }
  void displayPixels(uint8_t* pixel_array, sf::RenderWindow& window) {
    window.clear();

    texture_from_pixels.update(pixel_array);
    window.draw(screen);

    window.display();
  }
};

float f(float x) { return x * x / (1 + x * x); }

int main() {
  //const int w = 400, h = 400;
  sf::Clock clock;

  sf::RenderWindow window(sf::VideoMode(w, h), "SFML works!");
  PixelDisplayer pixel_d(w,h);
  OpenSimplexNoise::Noise noise(chrono::steady_clock::now().time_since_epoch().count());
  
  uint8_t* pixels = new uint8_t[w * h * 4];

  PetriDish dish(w,h);
  updateStructure reaction;
  reaction.diffusion_rates = Cell(10);
  // Fisher reaction
  // reaction.feedRates = [](const Cell& c) -> Cell {return c * (Cell(1,1) - c);};
  // reaction.reaction  = [](const Cell& c) -> Cell {return Cell(0, 0);};

  // FitzHugh-Nagumo
  // reaction.feedRates = [](const Cell& c) -> Cell {return Cell(0, 0);};
  // reaction.reaction  = [](const Cell& c) -> Cell {return Cell(10 * (c.a - c.a * c.a * c.a - c.b), c.a);};

  // Simple Limit Cycle
  reaction.feedRates = [](const Cell& c) -> Cell {return Cell(0, 0);};
  reaction.reaction  = [](const Cell& c) -> Cell {return c * ( 0.7 - (pow(c.a,2) + pow(c.b,2)) ) + Cell(-c.b,c.a);};

  // Set initial conditions
  for (int x = 0; x < w; ++x) {
    for (int y = 0; y < h; ++y) {
      Cell& cell = dish.getCellAt(x,y);
      cell = Cell(noise.eval(x/10.0, y/10.0), noise.eval(x/10.0, y/10.0, 0)) ;
      //cell = Cell( sin(-4000 * (x+1) / ((x+1)*(x+1) + (y+1)*(y+1)) ) ,
      //             cos(-4000 * (y+1) / ((x+1)*(x+1) + (y+1)*(y+1)) ) );

      //cell.a = x>200 ? 0.5 : 0;//f(x - sin(x/20) * y);
      //cell.b = y > 200 ? 1 : 0.2;//f(x * y - sin(y/25) * tan(y/5) * y);

      // cell[not (190 < x and x < 200 and 140 < y and y < 150)] = 1;
      // cell.b = 0;
    }
  }

  uint32_t c = 0;

  while (window.isOpen()) {//float t = clock.restart().asSeconds();
        //window.setTitle(to_string(1/t));
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        window.close();
    }

    // Move origin of shape with mouse
    //sf::Vector2i position = sf::Mouse::getPosition(window);
    //shape.setOrigin(-position.x, -position.y);

    // pixel_array test
    /*
    for (int x = 0; x < w; ++x) {
      for (int y = 0; y < h; ++y) {
        int pixel_index = 4 * (x * h + y);
        // R G B A
        pixels[pixel_index + 0] = c & 0xFF;
        pixels[pixel_index + 1] = (y > 200) ? c*c : 0;
        pixels[pixel_index + 2] = (x * x - 200 * y > 0) ? c*c*c : 0;
        pixels[pixel_index + 3] = 255;
      }
    }*/
    /*float noise_freq = 5, noise_scale = 50, noise_amplitude = 0.2;
    auto height_map = [=] (int x, int y, int t) -> float {
      float height = 0, scale = noise_scale, amplitude = 1;
      for (int i = 0; i < 8; ++i) {
        height += amplitude * noise.eval(x / scale, y / scale, t);
        scale *= noise_freq;
        amplitude *= noise_amplitude;
      }
      return height * (1 - noise_amplitude);
    };
    for (int x = 0; x < w; ++x) {
      for (int y = 0; y < h; ++y) {
        int pixel_index = 4 * (x * h + y);
        // R G B A
        pixels[pixel_index + 0] = (height_map(x , y , c/10.0) + 1) / 2.0 * 0xFF;
        pixels[pixel_index + 1] = (height_map(x , y , c/10.0) + 1) / 2.0 * 0xFF;
        pixels[pixel_index + 2] = (height_map(x , y , c/10.0) + 1) / 2.0 * 0xFF;
        pixels[pixel_index + 3] = 0xFF;
      }
    }*/
    c++;

    // Save image from pixel array
    //img.create(w,h,pixels);
    //img.saveToFile("image.png");
    
    //dish_draw.setTexture(dish);
    //shape.setTexture(&dish);
    //window.draw(dish_draw);
    dish.pushToPixels(pixels);
    
    pixel_d.displayPixels(pixels, window);

    //window.setTitle(to_string(c));

    dish.updateDish(0.1, reaction);

  }
  float t = clock.restart().asSeconds();
  cout << "frame count = " << int(c) << '\n';
  cout << "time = " << t << '\n';
  cout << "fps = " << int(c) / t << '\n';
  cout << "hello\n";
  return 0;
}