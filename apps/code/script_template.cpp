#include "script_template.h"

namespace Code {

constexpr ScriptTemplate emptyScriptTemplate(".py", "\x01" R"(from math import *
)");
constexpr ScriptTemplate tablesScriptTemplate("tables.py", R"( from random import *
def tables(tbl="",s="12345"):
  seed(int(s))
  if tbl=="2**x": m,M,n,N,rt=2,2,0,10,"^"
  elif tbl=="x**2": m,M,n,N,rt=0,20,2,2,"^"
  elif tbl=="a*b": m,M,n,N,rt=0,20,0,20,"x"
  br,mr,tx=0,0,0
  while tx<=0.9 or br<=10:
    a=randint(m,M)
    b=randint(n,N)
    if rt=="^": c=a**b
    elif rt=="x": c=a*b
    x=int(input(f"{a}{rt}{b}="))
    if x==c:
      br+=1
      print("Oui!")
    else:
      mr+=1
      z=0
      print("NON! essaie de nouveau...")
      while x!=c and z<=5:
        x=int(input(f"{a}{rt}{b}="))
        z+=1
      if x==c: print("Bravo! on continue...")
      else: print(f"NON ({a}{rt}{b}={c}) ")
    tx=br/(br+mr)
  print(f"{br} bonnes reponses sur {br+mr}")
)");


constexpr ScriptTemplate fractionsScriptTemplate("affichage_fractions.py", R"( from math import *
def decimal(a,b):
    quotient=str(a//b)  #la partie entiere du quotient
    reste=a%b
    texte=0
    if reste!=0:
        suiteRestes=[reste]
        quotient+=","
        while texte==0:
            quotient+=str(reste*10//b)
            reste=(reste*10)%b
            suiteRestes.append(reste)
            if reste==0:
                return quotient
            for i in range(len(suiteRestes)-1) :
                if reste==suiteRestes[i] :
                    return '{}/{}'.format(a,b)
    return quotient
)");


constexpr ScriptTemplate equationsScriptTemplate("equations.py", R"( from math import *
from random import randint

def second_degre(a,b,c):  #équation de la forme ax^2+bx+c=0
    delta=b**2-4*a*c
    if delta==0:
        print("Delta=",delta,"\tx=",(-b)/(2*a))
    elif delta>0:
        x1=(-b-sqrt(delta))/(2*a)
        x2=(-b+sqrt(delta))/(2*a)
        print("Delta=",delta,"\tx1=",x1,"\tx2=",x2)
    else:
        z1=complex((-b)/(2*a),-((sqrt(abs(delta)))/(2*a)))
        z2=complex((-b)/(2*a),(sqrt(abs(delta)))/(2*a))
        print("Delta=",delta,"\tx1=",z1,"\tx2=",z2)


def premier_degre(a,b,c):  #équation de la forme ax+b=c
    if a==0:
        if b==c:
            print("Tous les réels sont solution.")
        else:
            print("Pas de solution.")
    else:
        print("La solution est:",(c-b)/a)


def carre(c):  #équation carrée de la forme x^2=c
    if c<0:
        print("Les solutions sont:",complex(0,-sqrt(-c)),"et",complex(0,sqrt(-c)))
    else:
        print("Les solutions sont:",-sqrt(c),"et",sqrt(c))


def prod_nul(a,b,c,d):  #équation produit nul de la forme (ax+b)(cx+d)=0
    if a==0 and c==0:
        print("Pas de solution")
    elif a==0:
        premier_degre(c*b,b*d,0)
    else:
        if c==0:
            premier_degre(a*d,b*d,0)
        else:
            print("Les solutions sont:",-b/a,"et",-c/d)


def FCT(e,x):  #Image de x par la fonction e
    a=e.format(x)
    return eval(a)



def Dicho(e,n,a,b): #équation par dichotomie en application du théorème des valeurs intermédiaires (TVI)
                    #e designe l'expression de la fonction dont on recherche les zéros
                    #n est le nombre de décimales souhaitées
                    #a et b sont les bornes de l'intervalle sur lequel on applique le TVI
    while b-a>10**(-n):
        x=a
        A=FCT(e,x)      #On utilise la fonction FCT qui renvoie l'image de x par la fonction e
        x=(a+b)/2
        B=FCT(e,x)
        if A*B<=0:
            b=(a+b)/2
        else:
            a=(a+b)/2
    print(round((a+b)/2,n),'est une solution de l'équation {}=0'.format(e))


def NbreDerive(e,a):  #Calcul du nombre dérivé en utilisant la définition du nombre dérivé de e en a
    h=0.001
    x=a
    A=FCT(e,x)
    x=a+h
    B=FCT(e,x)
    return (B-A)/h   #On assimile le nombre dérivé de e en a à la valeur de (e(a+h)-e(a))/h


def DichoTang(e,a):      #On applique la methode de la tangente
    u=a                  #e est l'expression de la fonction, a est l'abscisse du point où l'on recherche le nombre dérivé
    for i in range(10):  #On suppose que 10 iterations suffisent pour obtenir une precision satisfaisante
        x=u
        F=FCT(e,x)
        G=NbreDerive(e,u)
        u-=F/G
    return u
)");


constexpr ScriptTemplate vecteursScriptTemplate("vecteurs.py", R"( from math import *

def coordonnees(xA,yA,xB,yB):
  return xB-xA,yB-yA

def egaux(xA,yA,xB,yB,xC,yC,xD,yD):
  (x1,y1)=coordonnees(xA,yA,xB,yB)
  (x2,y2)=coordonnees(xC,yC,xD,yD)
  return x1==x2 and y1==y2

def collineaires(xA,yA,xB,yB,xC,yC,xD,yD):
  (x1,y1)=coordonnees(xA,yA,xB,yB)
  (x2,y2)=coordonnees(xC,yC,xD,yD)
  print(x1*y2-x2*y1==0,": determinant =",x1*y2-x2*y1)

def equation_cartesienne_ab(xa,ya,xb,yb):
  b=xa-xb
  a=yb-ya
  c=-a*xa-b*ya
  print(a,"x+",b,"y+",c,"=0")

def equation_cartesienne_au(xa,ya,xu,yu):
  b=-xu
  a=yu
  c=-a*xa-b*ya
  print(a,"x+",b,"y+",c,"=0")

def longueurAB(xA,yA,xB,yB):
  distcarre=(xB-xA)**2+(yB-yA)**2
  return distcarre,sqrt(distcarre)

def parallelogramme(xA,yA,xB,yB,xC,yC,xD,yD):
  return egaux(xA,yA,xB,yB,xD,yD,xC,yC)

def alignes(xA,yA,xB,yB,xC,yC):
  (x1,y1)=coordonnees(xA,yA,xB,yB)
  (x2,y2)=coordonnees(xA,yA,xC,yC)
  print(x1*y2-x2*y1==0,": determinant =",x1*y2-x2*y1)
)");


constexpr ScriptTemplate snakeScriptTemplate("snake.py", R"( from random import randint
from kandinsky import color,draw_string,fill_rect
from ion import *
from time import *

s=None

#125 30/09
#84 11/10
#81 18/09

#94 13/10 Thibault

def dur(t):
  if t<0:
    t=0
  return t

class Snake:
  def __init__(self,couleur=(0,255,0)):
    self.x=randint(0,10)
    self.y=10
    self.couleur=couleur
    self.orientation=(1,0)
    self.queue=[(-1,-1)]
    self.mort=False
    self.raison="Je ne sait pas pourquoi!!"
    self.pas=0

  def orienter(self):
    ex_orientation=self.orientation
    for i in range(4):
      if keydown(i):
        self.orientation=((-1,0),(0,-1),(0,1),(1,0))[i]
    if self.orientation==(-ex_orientation[0],-ex_orientation[1]):
      self.orientation=ex_orientation

  def bouger(self):
    self.pas+=1
    if len(self.queue)>1:
      i=len(self.queue)
      while i>0:
        self.queue[i-1]=self.queue[i-2]
        i+=-1
    self.queue[0]=(self.x,self.y)
    self.x+=self.orientation[0]
    self.y+=self.orientation[1]
    if not 0<self.x<32 or not 0<self.y<22:
      self.raison="Tu a touche un mur."
      self.mort=True
    for p in pommes:
      if self.x==p.x and self.y==p.y:
        p.bouffer(self)
    if (self.x,self.y) in self.queue:
      self.raison="Tu t'est recoupe."
      self.mort=True


  def draw(self):
    last=self.queue[-1]
    if type(self.couleur)==type([2,3]):
      fill_rect(self.x*10,self.y*10,10,10,self.couleur[self.pas%len(self.couleur)])
    else:
      fill_rect(self.x*10,self.y*10,10,10,self.couleur)
    fill_rect(last[0]*10,last[1]*10,10,10,couleur_fond)
    for p in pommes:
      if p.x==last[0] and p.y==last[1]:
        p.draw()

  def draw_all(self):
    if type(self.couleur)==type([2,3]):
      for i in self.queue:
        fill_rect(i[0]*10,i[1]*10,10,10,self.couleur[0])
    else:
      for i in self.queue:
        fill_rect(i[0]*10,i[1]*10,10,10,self.couleur)
    self.draw()

  def pv(self,n):
    if n>0:
      try:
        for i in range(n):
          self.queue.append((-1,-1))
      except:
        pass
    if n<0:
      for i in range(-n):
        if not len(self.queue)<=1:
          fill_rect(self.queue[-1][0]*10,self.queue[-1][1]*10,10,10,couleur_fond)
          del self.queue[-1]
    draw_string("Score:"+str(len(self.queue))+" ",10,10)


class Pomme:
  def __init__(self,couleur=(255,0,0),pv=1,clonage=1,autres=[],tp=1):
    self.couleur=couleur
    self.pv=pv
    self.clonage=clonage
    self.autres=autres
    self.tp=tp
    self.novCoord()
    try:
      pommes.append(self)
    except:
      pass

  def bouffer(self,serpent):
    global pommes
    if self.pv==-1000:
      serpent.mort=True
      serpent.raison="Vous avez mangee \n\tune pomme pourrie."
      return
    serpent.pv(self.pv)
    if self.tp!=1:
      serpent.x+=serpent.orientation[0]*self.tp
      serpent.y+=serpent.orientation[1]*self.tp
      fill_rect(self.x*10,self.y*10,10,10,(255,255,255))
    for i in range(self.clonage):
      Pomme(self.couleur,self.pv,self.clonage,self.autres,self.tp)
      pommes[-1].draw()
    for i in self.autres:
      for j in range(i[0]):
        Pomme(i[1],i[2],i[3])
        pommes[-1].draw()

    pommes.remove(self)
    del self
    return

  def novCoord(self):
    x=2+randint(0,28)
    y=2+randint(0,18)
    for i in pommes:
      if (x,y)==(i.x,i.y):
        self.novCoord()
        return
    self.x=x
    self.y=y

  def draw(self):
    fill_rect(self.x*10,self.y*10,10,10,self.couleur)

def draw_all():
  global s
  fill_rect(0,0,320,222,couleur_fond)
  s.draw_all()
  for p in pommes:
    p.draw()
  draw_string("Score:"+str(len(s.queue))+" ",10,10)

def commencer(m=((1,(255,0,0),1,1),),c=(0,255,0)):
  global s
  s=Snake(c)
  mode_pomme(m)

  draw_all()

def perdre(serpent):
  score=len(serpent.queue)
  draw_string("Perdu!!",100,70)
  draw_string(serpent.raison,60,100)
  draw_string("Score:"+str(score),80,140)
  print("Score:"+str(score))
  return(score)

couleur_fond=(255,255,255)

pommes=[]

fps=1
vitesse=1/fps

def mode_pomme(quoi):
  global pommes
  pommes=[]
  for type in quoi:
    if len(type)==4:
      for n in range(type[0]):
        Pomme(couleur=type[1],pv=type[2],clonage=type[3])
    if len(type)==5:
      for n in range(type[0]):
        Pomme(couleur=type[1],pv=type[2],clonage=type[3],autres=type[4])
    if len(type)==6:
      for n in range(type[0]):
        Pomme(couleur=type[1],pv=type[2],clonage=type[3],autres=type[4],tp=type[5])

def boucler(vitesse=vitesse):
  while True:
    t=monotonic()
    s.orienter()
    s.bouger()
    s.draw()
    if s.mort:
      return(perdre(s))
      break
    if keydown(KEY_ONOFF):
      fill_rect(0,0,320,222,(0,0,0))
      sleep(0.25)
      while not keydown(KEY_ONOFF):
        sleep(0.1)
      draw_all()
      sleep(0.25)
    sleep(dur(t-monotonic()+vitesse))

def start():
  commencer()
  boucler()
  draw_string("Ressuciter: touche <=",80,160)
  while not keydown(KEY_BACKSPACE):
    sleep(0.05)
  start()



pommes_bouffe=0
niv=0
coefficient=2
b_score=0
score=0
mode_selectionne=0
marge_erreur=0
produit=0
s=None
KEY_QUIT=KEY_BACKSPACE
KEY_SHOP=KEY_SHIFT


mode_possible=(
((1,(255,0,0),1,1),), #best 125
((1,(170,0,0),-1,1),(2,(170,0,0),3,1)),
((1,(255,0,255),7,1,[(1,(50,50,50),-1000,1),]),),
((1,(255,0,0),1,2),(1,(255,200,0),5,1)),
((2,(0,255,255),8,1,[],3),),#best 192
((1,(255,0,0),1,1),(1,(170,0,0),-1,1),(2,(170,0,0),3,1),(1,(255,0,255),7,1,[(1,(0,0,0),-1000,1),]),(1,(255,0,0),1,2),(1,(255,200,0),5,1),(2,(0,255,255),8,1,[],3))
)

base_produits=[
[(0,255,0),0,True],
[(0,255,255),10,False],
[(0,0,255),20,False],
[(255,0,255),30,False],
[(255,0,0),40,False],
[(255,255,0),50,False],
[(0,0,0),80,False],
[[(0,250,0),(0,200,0),(0,150,0),(0,100,0),(0,150,0),(0,200,0),],160,False],
[[(0,250,250),(0,200,200),(0,150,150),(0,100,100),(0,150,150),(0,200,200),],180,False],
[[(0,0,250),(0,0,200),(0,0,150),(0,0,100),(0,0,150),(0,0,200),],200,False],
[[(250,0,250),(200,0,200),(150,0,150),(100,0,100),(150,0,150),(200,0,200),],220,False],
[[(255,0,0),(200,0,0),(150,0,0),(100,0,0),(150,0,0),(200,0,0)],240,False],
[[(250,250,0),(200,200,0),(150,150,0),(100,100,0),(150,150,0),(200,200,0),],260,False],
[[(0,0,0),(50,50,50),(100,100,100),(150,150,150),(100,100,100),(50,50,50),],280,False],
[[(0,255,0),(0,255,255),(0,0,255),(255,0,255),(255,0,0),(255,255,0)],500,False]
]


def cadenas(x,y,text,couleur=(50,50,50),condition=False):

  fill_rect(x+15,y+5,20,10,couleur)

  fill_rect(x+30,y+10,10,15,couleur)
  fill_rect(x+10,y+10,10,8,couleur)

  if not condition:
    fill_rect(x+10,y+18,10,7,couleur)

  fill_rect(x+5,y+25,40,25,couleur)

  draw_string(text,x+25-(5*len(text)),y+30,(255,255,255),couleur)


def man():
  global niv
  niv=int(pommes_bouffe**(1/coefficient))


def draw(arg=""):
  fill_rect(0,0,320,222,(255,255,255))

  draw_string("Niveau "+str(niv),100,10)

  fill_rect(68,28,154,14,(200,200,200))
  fill_rect(70,30,int(150*((pommes_bouffe-niv**coefficient)/((niv+1)**coefficient-niv**coefficient))),10,(240,0,0))

  fill_rect(265,15,40,30,(255,0,0))
  draw_string(str(pommes_bouffe),285-(5*len(str(pommes_bouffe))),20,(0,0,0),(255,0,0))

  if arg=="only_score":
    return

  draw_string("Score: "+str(score),80,60)
  draw_string("Meilleur score: "+str(b_score),80,80,(255*(b_score==score),0,0))

  draw_string("Shop: Shift",80,110)
  draw_string("Jouer: Ok",80,130)
  draw_string("Quitter: <X",80,150)

  fill_rect(10+mode_selectionne*50,172,50,50,(200,200,200))
  for i in range(6):
    cadenas(10+i*50,172,str(i*5),condition=niv>=i*5)


def saveConf(pommes_bouffe,b_score,produits):
  try :
    with open("the_snake_game.conf","w") as f:
      f.truncate(0)
      f.write(str([pommes_bouffe,b_score,produits]))
  except:
    print("Saving configuration failed.")

def loadConf():
  try:
    with open("the_snake_game.conf","r") as f:
      return eval(f.readline())
  except:
    print("Loading configuration failed.")
    return 0,0,base_produits


pommes_bouffe,b_score,produits=loadConf()
man()
while True:
  try:
    draw()
    sleep(0.5)

    while not(keydown(KEY_OK) or keydown(KEY_QUIT) or keydown(KEY_SHOP)):
      if keydown(KEY_VAR):
        pommes_bouffe=0
        niv=0
        coefficient=2
        b_score=0
        score=0
        produits=base_produits
        produit=0
        saveConf(0,0,base_produits)
        mode_selectionne=0
        break
      if keydown(KEY_PI):
        pommes_bouffe+=10
        man()
        draw()
      for i in range(12,18):
        if keydown(i) and niv>=(i-12)*5:
          mode_selectionne=i-12
          draw()
      if keydown(0) or keydown(3):
        mode_selectionne+=keydown(3)-keydown(0)
        mode_selectionne%=int(niv/5)+1
        if mode_selectionne>5:
          mode_selectionne=5
        draw()
      sleep(0.05)

    if keydown(KEY_QUIT):
      saveConf(pommes_bouffe,b_score,produits)
      print("Total des pommes mangee: "+str(pommes_bouffe))
      break

    if keydown(KEY_SHOP):
      while keydown(KEY_SHOP):
        pass

      while not keydown(KEY_SHOP):

        draw("only_score")
        fill_rect(15+60*(produit%5),47+60*(produit//5),56,56,(200,200,200))

        for i in range(len(produits)):

          fill_rect(18+60*(i%5),50+60*(i//5),50,50,(220,220,100+120*(not produits[i][2])))

          draw_string(str(produits[i][1]),35+60*(i%5),80+60*(i//5))

          if type(produits[i][0])==type([2,3]):
            for j in range(6):
              fill_rect(20+[0,0,0,10,20,20][j]+(i%5)*60,50+60*(i//5)+[20,10,0,0,0,10][j],10,10,produits[i][0][j%len(produits[i][0])])

          if type(produits[i][0])==type((255,0,4)):
            for j in range(6):
              fill_rect(20+[0,0,0,10,20,20][j]+(i%5)*60,50+60*(i//5)+[20,10,0,0,0,10][j],10,10,produits[i][0])

        while not (keydown(0) or keydown(1) or keydown(2) or keydown(3) or keydown(KEY_OK)):
          pass

        if keydown(0) or keydown(3):
          produit+=keydown(3)-keydown(0)

        elif keydown(1) or keydown(2):
          produit+=5*(keydown(2)-keydown(1))

        produit%=len(produits)

        if keydown(KEY_OK):
          if pommes_bouffe>=produits[produit][1] and produits[produit][2]!=True:
            pommes_bouffe-=produits[produit][1]
            produits[produit][2]=True
            man()
            mode_selectionne=0
        sleep(0.1)
      sleep(0.2)
      continue

    commencer(m=mode_possible[mode_selectionne],c=produits[produit*(produits[produit][2])][0])
    draw_all()
    score=boucler(0.1)

    while not(keydown(KEY_OK) or keydown(KEY_QUIT)):
      sleep(0.05)
    pommes_bouffe+=score
    if score>b_score:
      b_score=score
    saveConf(pommes_bouffe,b_score,produits)
    man()

  except KeyboardInterrupt:
    saveConf(pommes_bouffe,b_score,produits)
    if marge_erreur>100:
      break
    else:
      marge_erreur+=1
)");



const ScriptTemplate * ScriptTemplate::Empty() {
  return &emptyScriptTemplate;
    return &emptyScriptTemplate;
}

const ScriptTemplate * ScriptTemplate::Tables() {
    return &tablesScriptTemplate;
}

const ScriptTemplate * ScriptTemplate::Fractions() {
    return &fractionsScriptTemplate;
}

const ScriptTemplate * ScriptTemplate::Equations() {
    return &equationsScriptTemplate;
}

const ScriptTemplate * ScriptTemplate::Vecteurs() {
    return &vecteursScriptTemplate;
}

const ScriptTemplate * ScriptTemplate::Snake() {
    return &snakeScriptTemplate;
}

}