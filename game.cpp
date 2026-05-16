#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
using namespace std;
 
void pause(){ cout << "\n  [ Press ENTER to continue ]\n"; cin.get(); }
 
class DifficultyManager {
    int day, total;
public:
    DifficultyManager(int t=10): day(1), total(t) {}
    void setDay(int d){ day=d; }
    int  getDay()  const { return day; }
 
    string phase() const {
        if(day<=3) return "Early Days  ( Safe-ish )";
        if(day<=6) return "Mid Game    ( Dangerous )";
        if(day<=8) return "Late Game   ( Critical )";
        return             "Apocalypse  ( Nightmare )";
    }
    int zombieHP()  const { return (day-1)*8; }
    int zombieDMG() const { return (day-1)*3; }
    bool isHorde()  const { return (rand()%100) < (day-1)*5; }
    int  hordeSize()const { return day<=4?2:day<=7?3:4; }
    int  foodLoot() const { return day<=3?2:day<=7?1:0; }
    int  escapePct()    const { return max(10, 70-(day-1)*6); }
    bool isQuietDay()   const { return (rand()%100) < max(5,35-(day-1)*3); }
    int  infectionPct() const { return day<=3?10:day<=6?30:day<=8?50:70; }
    int  starveDMG()    const { return 8+(day-1)*2; }
 
    void showBanner() const {
        cout << "\n";
        cout << "  DAY " << day << " of " << total << "   |   " << phase() << "\n";
        cout << "\n";
        cout << "  Zombie Bonus  :  +" << zombieHP() << " HP   +" << zombieDMG() << " DMG\n";
        cout << "  Escape Chance :   " << escapePct() << "%\n";
        cout << "  Infection Risk:   " << infectionPct() << "%\n";
        cout << "\n";
    }
};
 
class StatusEffect {
    bool infected;
    int  days;
public:
    StatusEffect(): infected(false), days(0) {}
    bool isInfected() const { return infected; }
 
    void apply() {
        if(!infected){
            infected=true; days=0;
            cout << "\n  ! INFECTED - You will lose HP each night. Find an antidote !\n";
        }
    }
 
    int nightlyDamage() {
        if(!infected) return 0;
        days++;
        int dmg = 8 + days*2;
        cout << "  Infection Day " << days << " :  -" << dmg << " HP tonight\n";
        return dmg;
    }
 
    int medkitHeal() const {
        int h = infected ? max(10, 40-days*10) : 40;
        if(infected) cout << "  Infection weakens medkit: +" << h << " HP only\n";
        return h;
    }
 
    void cure(){ infected=false; days=0; cout << "  Infection cured!\n"; }
 
    void show() const {
        cout << "  Status   :  " << (infected ? "INFECTED (day " + to_string(days) + ")" : "Healthy") << "\n";
    }
};
 
class Weapon {
    string name; int dmg, ammo; bool melee;
public:
    Weapon(string n, int d, int a=-1): name(n),dmg(d),ammo(a),melee(a==-1){}
    string getName()  const { return name; }
    bool   canFire()  const { return melee||ammo>0; }
    void   addAmmo(int n)   { if(!melee) ammo+=n; }
 
    int fire(){
        if(!canFire()){ cout << "  " << name << " is out of ammo!\n"; return 0; }
        if(!melee) ammo--;
        return dmg+(rand()%6);
    }
    void show() const {
        cout << "  " << name << "   DMG: " << dmg
             << "   Ammo: " << (melee ? "unlimited" : to_string(ammo)) << "\n";
    }
};
 
class Inventory {
    int food, medkits, antidotes;
    vector<Weapon> weapons;
    int cur;
public:
    Inventory(): food(4),medkits(2),antidotes(0),cur(0){
        weapons.push_back(Weapon("Knife",15));
        weapons.push_back(Weapon("Pistol",35,6));
    }
    Weapon& weapon()   { return weapons[cur]; }
    int getFood()      const { return food; }
    int getMedkits()   const { return medkits; }
    int getAntidotes() const { return antidotes; }
    void addFood(int n)     { food+=n; }
    void addMedkit(int n)   { medkits+=n; }
    void addAntidote(int n) { antidotes+=n; }
    void addWeapon(Weapon w){ weapons.push_back(w); }
    bool useFood()     { if(!food)     return false; food--;      return true; }
    bool useMedkit()   { if(!medkits)  return false; medkits--;   return true; }
    bool useAntidote() { if(!antidotes)return false; antidotes--; return true; }
 
    void switchWeapon(){
        if((int)weapons.size()<=1){ cout << "  You only have one weapon.\n"; return; }
        cur=(cur+1)%(int)weapons.size();
        cout << "  Switched to: " << weapons[cur].getName() << "\n";
    }
    void show() const {
        cout << "  Food: " << food
             << "   Medkits: " << medkits
             << "   Antidotes: " << antidotes << "\n\n";
        cout << "  Weapons:\n";
        for(int i=0;i<(int)weapons.size();i++){
            cout << (i==cur ? "  > " : "    ");
            weapons[i].show();
        }
    }
};
 
class Entity {
protected:
    string name; int hp, maxhp;
public:
    Entity(string n,int h): name(n),hp(h),maxhp(h){}
    virtual ~Entity(){}
    string getName()   const { return name; }
    int    getHP()     const { return hp; }
    bool   isAlive()   const { return hp>0; }
    void takeDamage(int d)   { hp-=d; if(hp<0) hp=0; }
    void heal(int h)         { hp+=h; if(hp>maxhp) hp=maxhp; }
    virtual void show()            const = 0;
    virtual int  getAttackDamage() const = 0;
};
 
class Player : public Entity {
    int day, totalDays;
    Inventory    inv;
    StatusEffect status;
public:
    Player(string n,int d=10): Entity(n,100),day(1),totalDays(d){}
    int  getDay()  const { return day; }
    bool hasWon()  const { return day>totalDays; }
    Inventory&    getInv()    { return inv; }
    StatusEffect& getStatus() { return status; }
    void nextDay() { day++; }
 
    void eat(int starveDmg){
        if(inv.useFood()){ heal(20); cout << "  You eat.  +20 HP\n"; }
        else { takeDamage(starveDmg); cout << "  No food! Starving!  -" << starveDmg << " HP\n"; }
    }
    void useMedkit(){
        if(!inv.useMedkit()){ cout << "  No medkits left!\n"; return; }
        int h=status.medkitHeal(); heal(h);
        cout << "  Medkit used.  +" << h << " HP\n";
    }
    void useAntidote(){
        if(!inv.useAntidote()){ cout << "  No antidotes left!\n"; return; }
        status.cure();
    }
    void processInfection(){ int d=status.nightlyDamage(); if(d>0) takeDamage(d); }
    int  attack()          { return inv.weapon().fire(); }
    void switchWeapon()    { inv.switchWeapon(); }
 
    void show() const override {
        cout << "\n";
        cout << "  Player   :  " << name << "   (Day " << day << " / " << totalDays << ")\n";
        cout << "  HP       :  " << hp << " / " << maxhp;
        if(hp>=70)      cout << "   [ Good ]\n";
        else if(hp>=40) cout << "   [ Caution ]\n";
        else if(hp>=20) cout << "   [ Danger ]\n";
        else            cout << "   [ Critical ]\n";
        status.show();
        cout << "\n";
        inv.show();
        cout << "\n";
    }
    int getAttackDamage() const override { return 15; }
};
 
class Zombie : public Entity {
protected:
    int atk; string type;
public:
    Zombie(string n,int h,int a,string t): Entity(n,h),atk(a),type(t){}
    void show() const override {
        cout << "  Zombie   :  " << name << "  [ " << type << " ]   HP: " << hp << " / " << maxhp << "\n";
    }
    int getAttackDamage() const override { return atk; }
    virtual int    attack()         = 0;
    virtual string describe() const = 0;
};
 
class SlowZombie : public Zombie {
public:
    SlowZombie(int hb,int db): Zombie("Rotten Shambler",50+hb,10+db,"Slow"){}
    int    attack()         override { return atk+(rand()%8); }
    string describe() const override { return "A slow shambling zombie approaches."; }
};
 
class FastZombie : public Zombie {
public:
    FastZombie(int hb,int db): Zombie("Sprinter",35+hb,20+db,"Fast"){}
    int attack() override {
        cout << "  Sprinter attacks twice!\n";
        return (atk+rand()%6)*2;
    }
    string describe() const override { return "A fast zombie - it attacks TWICE per turn!"; }
};
 
class BossZombie : public Zombie {
    bool rage;
public:
    BossZombie(int hb,int db): Zombie("Mutant Boss",110+hb*2,28+db,"Boss"),rage(false){}
    int attack() override {
        if(hp<maxhp/2 && !rage){ rage=true; cout << "\n  ! Boss enters RAGE MODE - damage doubled !\n"; }
        return (atk+rand()%12)*(rage?2:1);
    }
    string describe() const override { return "A massive boss zombie. It rages at half HP!"; }
};
 
class Game {
    Player*           player;
    DifficultyManager diff;
 
    Zombie* spawn(){
        int h=diff.zombieHP(), d=diff.zombieDMG(), r=rand()%10;
        if(diff.getDay()>=7 && r>=8) return new BossZombie(h,d);
        if(diff.getDay()>=4 && r>=5) return new FastZombie(h,d);
        return new SlowZombie(h,d);
    }
 
    void fight(Zombie* z){
        cout << "\n  " << z->describe() << "\n";
        z->show();
        pause();
 
        while(player->isAlive() && z->isAlive()){
            player->show();
            cout << "  Actions:\n";
            cout << "    1. Attack\n";
            cout << "    2. Switch Weapon\n";
            cout << "    3. Use Medkit\n";
            cout << "    4. Use Antidote\n";
            cout << "    5. Run  ( " << diff.escapePct() << "% chance )\n";
            cout << "\n  Choice: ";
            int c; cin>>c; cin.ignore(); cout << "\n";
 
            if(c==1){
                int dmg=player->attack();
                if(dmg>0){ z->takeDamage(dmg); cout << "  You hit for " << dmg << " damage!\n"; }
            }
            else if(c==2) player->switchWeapon();
            else if(c==3) player->useMedkit();
            else if(c==4) player->useAntidote();
            else if(c==5){
                if((rand()%100)<diff.escapePct()){ cout << "  You escaped!\n"; pause(); return; }
                else cout << "  Escape blocked!\n";
            }
            else{ cout << "  Invalid choice.\n"; continue; }
 
            if(z->isAlive()){
                int zdmg=z->attack();
                player->takeDamage(zdmg);
                cout << "  " << z->getName() << " hits you for " << zdmg << " damage!\n";
                if((rand()%100)<diff.infectionPct())
                    player->getStatus().apply();
            }
            z->show();
            pause();
        }
        if(!z->isAlive()){ cout << "\n  " << z->getName() << " is dead.\n"; loot(); }
    }
 
    void loot(){
        int r=rand()%5; Inventory& inv=player->getInv();
        cout << "  Searching the body...  ";
        if(r==0){ int f=diff.foodLoot(); if(f>0){inv.addFood(f); cout<<"Found "<<f<<" food!\n";}
                  else cout<<"No food found.\n"; }
        else if(r==1){ inv.addMedkit(1);   cout << "Found a Medkit!\n"; }
        else if(r==2){ inv.addAntidote(1); cout << "Found an Antidote!\n"; }
        else if(r==3){ inv.addWeapon(Weapon("Shotgun",60,3)); cout << "Found a Shotgun!\n"; }
        else           cout << "Nothing.\n";
    }
 
    void scavenge(){
        cout << "\n  Searching nearby buildings...\n";
        int r=rand()%4; Inventory& inv=player->getInv(); int f=diff.foodLoot();
        if(r==0){ if(f>0){inv.addFood(f); cout<<"  Found "<<f<<" food!\n";}
                  else cout<<"  No food left anywhere.\n"; }
        else if(r==1){ inv.addMedkit(1);   cout << "  Found a Medkit!\n"; }
        else if(r==2){ inv.addAntidote(1); cout << "  Found an Antidote!\n"; }
        else           cout << "  Nothing useful found.\n";
        pause();
    }
 
    void horde(){
        int n=diff.hordeSize();
        cout << "\n  ZOMBIE HORDE INCOMING  -  " << n << " zombies!\n";
        pause();
        for(int i=0;i<n&&player->isAlive();i++){
            cout << "\n  Zombie " << (i+1) << " of " << n << ":\n";
            Zombie* z=spawn(); fight(z); delete z;
        }
    }
 
    void dayEvent(){
        if(diff.isQuietDay()){
            cout << "  Quiet day. You rest.\n";
            player->heal(15);
            cout << "  +15 HP\n";
            pause(); return;
        }
        if(diff.isHorde()){ horde(); return; }
        int r=rand()%5;
        if(r<=2){ cout << "\n  Groaning nearby...\n"; Zombie* z=spawn(); fight(z); delete z; }
        else if(r==3) scavenge();
        else{ cout << "\n  All quiet. Nothing happens.\n"; pause(); }
    }
 
    void nightPhase(){
        cout << "\n";
        cout << "  Night falls.  Day " << player->getDay() << " is over.\n";
        cout << "\n";
        player->processInfection();
        player->eat(diff.starveDMG());
        pause();
        player->nextDay();
    }
 
public:
    Game(): player(nullptr), diff(10){}
    ~Game(){ delete player; }
 
    void start(){
        srand((unsigned)time(0));
 
        cout << "\n";
        cout << "  ZOMBIE SURVIVAL\n";
        cout << "  Survive 10 days. Difficulty increases each day.\n";
        cout << "\n";
        cout << "  Your name: ";
        string n; cin>>n; cin.ignore();
 
        player = new Player(n,10);
 
        cout << "\n";
        cout << "  Welcome, " << n << ".\n";
        cout << "  Days 1-3 are manageable. By Day 7 you will be fighting for your life.\n";
        pause();
 
        while(player->isAlive() && !player->hasWon()){
            diff.setDay(player->getDay());
            diff.showBanner();
            dayEvent();
            if(!player->isAlive()) break;
            nightPhase();
        }
 
        cout << "\n";
        if(player->hasWon())
            cout << "  YOU SURVIVED 10 DAYS.  A rescue helicopter arrives.\n";
        else
            cout << "  GAME OVER.  You fell on Day " << player->getDay() << ".\n";
        cout << "\n";
        player->show();
    }
};
 
int main(){ Game g; g.start(); return 0; }