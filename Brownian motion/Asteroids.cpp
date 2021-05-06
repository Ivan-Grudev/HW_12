#include <SFML/Graphics.hpp>
#include <time.h>
#include <list>
#include <random>
#include <iostream>

using namespace sf;

const int W = 1200;
const int H = 800;

float DEGTORAD = 0.017453f;

class Animation
{
   public:
   float Frame, speed;
   Sprite sprite;
   std::vector<IntRect> frames;

   Animation(){}

   Animation (Texture &t, int x, int y, int w, int h, int count, float Speed)
   {
     Frame = 0;
     speed = Speed;

     for (int i=0;i<count;i++)
      frames.push_back( IntRect(x+i*w, y, w, h)  );

     sprite.setTexture(t);
     sprite.setOrigin(w/2,h/2);
     sprite.setTextureRect(frames[0]);
   }


   void update()
   {
     Frame += speed;
     int n = frames.size();
     if (Frame >= n) Frame -= n;
     if (n>0) sprite.setTextureRect( frames[int(Frame)] );
   }

   bool isEnd()
   {
     return Frame+speed>=frames.size();
   }

};


class Entity
{
  public:
   float x, y, dx, dy, R, angle;

   int life;

   std::string name;
   Animation anim;

    Entity()
    {
        life = 1;
    }

   void settings(Animation &a, int X, int Y, float Angle = 0, int radius = 1)
   {
     anim = a;
     x=X; y=Y;
     angle = Angle;
     R = radius;
   }

   virtual void update(){};

   void draw(RenderWindow &app)
   {
     anim.sprite.setPosition(x,y);
     anim.sprite.setRotation(angle+90);
     app.draw(anim.sprite);

     CircleShape circle(R);
     circle.setFillColor(Color(255,0,0,170));
     circle.setPosition(x,y);
     circle.setOrigin(R,R);
     //app.draw(circle);
   }

   virtual ~Entity(){};
};


class asteroid: public Entity
{
   public:
   asteroid()
   {
     dx=rand()%8-4;
     dy=rand()%8-4;
     name="asteroid";
   }

   void update()
   {
     x+=dx;
     y+=dy;

     if (x > W - 5 || x < 5) dx = -dx;
     if (y > H - 5 || y < 5) dy = -dy;  
   }

};


class bullet: public Entity
{
   public:
   bullet()
   {
     name="bullet";
   }

   void  update()
   {
     dx=cos(angle*DEGTORAD)*6;
     dy=sin(angle*DEGTORAD)*6;
     // angle+=rand()%7-3;  /*try this*/
     x+=dx;
     y+=dy;

     if (x>W || x<0 || y>H || y<0) life=0;
   }

};

class player: public Entity
{
   public:
   bool thrust;
   int health;
   int score;

   player()
   {
     name="player";
     life = 3;
     health = 100;
     score = 0;
   }

   void update()
   {
     if (thrust)
      { dx+=cos(angle*DEGTORAD)*0.2;
        dy+=sin(angle*DEGTORAD)*0.2; }
     else
      { dx*=0.99;
        dy*=0.99; }

    int maxSpeed=15;
    float speed = sqrt(dx*dx+dy*dy);
    if (speed>maxSpeed)
     { dx *= maxSpeed/speed;
       dy *= maxSpeed/speed; }

    x+=dx;
    y+=dy;

    if (x>W) x=0; if (x<0) x=W;
    if (y>H) y=0; if (y<0) y=H;
   }

};


bool isCollide(Entity *a,Entity *b)
{
  return (b->x - a->x)*(b->x - a->x)+
         (b->y - a->y)*(b->y - a->y)<
         (a->R + b->R)*(a->R + b->R);
}

bool isInCell(int i, int j, Entity *a)
{
    if (a->x >= 200 * (i - 1) && a->x <= 200 * i && a->y >= 200 * (j - 1) && a->y <= 200 * j) 
        return true;

    return false;
}

int main()
{
    srand(time(0));

    RenderWindow app(VideoMode(W, H), "Brownian motion");
    app.setFramerateLimit(60);

    Texture t2,t4;
    t2.loadFromFile("images/background.jpg");
    t4.loadFromFile("images/rock.png");
    //t4.loadFromFile("images/particle.png");

    t2.setSmooth(true);

    Sprite background(t2);

    Animation sRock(t4, 0, 0, 64, 64, 16, 0.2);

    std::list<Entity*> entities;

    for(int i=0; i < 45; i++)
    {
      asteroid *a = new asteroid();
      a->settings(sRock, rand()%W, rand()%H, rand()%360, 5);
      //a->settings(sRock, rand()%W, rand()%H, 0, 5);
      entities.push_back(a);
    }

    /////main loop/////
    while (app.isOpen())
    {
        Event event;
        while (app.pollEvent(event))
        {
            if (event.type == Event::Closed)
                app.close();
        }

        for (auto a : entities)
            for (auto b : entities)
            {
                if (isCollide(a, b))
                {
                    //здесь будет сценарий столкновения частиц
                }
            }

        for (auto i = entities.begin(); i != entities.end();)
        {
            Entity* e = *i;

            e->update();
            e->anim.update();
            i++;
        }

        //////draw//////
        app.draw(background);

        RectangleShape line_h(Vector2f(1200.f, 2.f));
        line_h.setFillColor(Color(15, 180, 140));

        RectangleShape line_v(Vector2f(800.f, 2.f));
        line_v.setFillColor(Color(15, 180, 140));
        line_v.rotate(90.f);

        for (int j = 1; j <= 4; j++)
        {
            for (int i = 1; i <= 6; i++)
            {
                int n = 0;

                for (auto a : entities)
                    if (isInCell(i, j, a))
                        n++;

                if (n > 3)
                {
                    RectangleShape cell(Vector2f(200.f, 200.f));
                    if (n < 5)
                        cell.setFillColor(Color(0, 180, 0, 75));
                    else if (n < 7)
                        cell.setFillColor(Color(150, 0, 180, 75));
                    else
                        cell.setFillColor(Color(180, 0, 0, 75));

                    if (i > 1 || j > 1)
                        cell.move(200 * (i - 1), 200 * (j - 1));

                    app.draw(cell);
                }
            }
        }

        for (auto i : entities) i->draw(app);

        for (int i = 0; i < 3; i++)
        {
            line_h.move(0, 200);

            // Отрисовка линии
            app.draw(line_h);
        }

        for (int i = 0; i < 5; i++)
        {
            line_v.move(200, 0);

            // Отрисовка линии
            app.draw(line_v);
        }

        app.display();
    }

    return 0;
}
