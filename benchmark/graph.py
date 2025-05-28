import csv
import matplotlib.pyplot as plt

def plot_nsquared():
  N_values = []
  naive_times = []
  rtree_times = []
  qtree_times = []

  with open("/Users/lucas/dev/rtree-tipe/result/benchmark_n2.csv", newline='') as csvfile:
      reader = csv.reader(csvfile, delimiter=';')
      next(reader)  # skip header
      for row in reader:
          N = int(row[0])
          naive = int(row[1])
          rtree = int(row[3])
          qtree = int(row[2])
          N_values.append(N)
          naive_times.append(naive)
          rtree_times.append(rtree)
          qtree_times.append(qtree)

  plt.figure(figsize=(10, 6))
  plt.plot(N_values, naive_times, label='Naive', marker='o')
  plt.plot(N_values, rtree_times, label='R-tree', marker='s')
  plt.plot(N_values, qtree_times, label='Quadtree', marker='s')
  plt.xlabel('N')
  plt.ylabel('Temps (ticks d\'horloge)')
  plt.title(r'Test $\mathcal{O}(n^2)$ (Calcul de distances moyennes)')
  plt.legend()
  plt.grid(True)
  plt.tight_layout()
  plt.show()

def plot_search():
  N_values = []
  naive_times = []
  rtree_times = []
  qtree_times = []

  with open("/Users/lucas/dev/rtree-tipe/result/benchmark_search.csv", newline='') as csvfile:
      reader = csv.reader(csvfile, delimiter=';')
      next(reader)  # skip header
      for row in reader:
          N = int(row[0])
          naive = int(row[1])
          rtree = int(row[3])
          qtree = int(row[2])
          N_values.append(N)
          naive_times.append(naive)
          rtree_times.append(rtree)
          qtree_times.append(qtree)

  plt.figure(figsize=(10, 6))
  plt.plot(N_values, naive_times, label='Naive', marker='o')
  plt.plot(N_values, rtree_times, label='R-tree', marker='s')
  plt.plot(N_values, qtree_times, label='Quadtree', marker='s')
  plt.xlabel('N')
  plt.ylabel('Temps (ticks d\'horloge)')
  plt.title(r'Test $\mathcal{O}(n)$ (Recherche)')
  plt.legend()
  plt.grid(True)
  plt.tight_layout()
  plt.show()

def plot_build():
  N_values = []
  rtree_times = []
  qtree_times = []

  with open("/Users/lucas/dev/rtree-tipe/result/benchmark_construction.csv", newline='') as csvfile:
      reader = csv.reader(csvfile, delimiter=';')
      next(reader)  # skip header
      for row in reader:
          N = int(row[0])
          rtree = int(row[2])
          qtree = int(row[1])
          N_values.append(N)
          rtree_times.append(rtree)
          qtree_times.append(qtree)

  plt.figure(figsize=(10, 6))
  plt.plot(N_values, rtree_times, label='R-tree', marker='s')
  plt.plot(N_values, qtree_times, label='Quadtree', marker='s')
  plt.xlabel('N')
  plt.ylabel('Temps (ticks d\'horloge)')
  plt.title(r'Construction ($\mathcal{O}(n)$)')
  plt.legend()
  plt.grid(True)
  plt.tight_layout()
  plt.show()

#plot_build()
#plot_search()
plot_nsquared()
