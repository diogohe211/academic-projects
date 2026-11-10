import random
import time

# -----------------------------
# Árvore Binária Genérica
# -----------------------------
class Node:
    def __init__(self, data):    
        self.data = data
        self.left = None
        self.right = None

class binTree:
    def __init__(self):
        self.root = None
    def contains(self, value):
        if self.root is None:
            return False
        queue = [self.root]
        while queue:
            node = queue.pop(0)
            if node.data == value:
                return True
            if node.left:
                queue.append(node.left)
            if node.right:
                queue.append(node.right)
        return False

    def insert(self, data):
        if self.contains(data):
            return  # não insere se já existir
        new_node = Node(data)
        if self.root is None:
            self.root = new_node
            return
        queue = [self.root]
        while queue:
            node = queue.pop(0)
            if node.left is None:
                node.left = new_node
                return
            else:
                queue.append(node.left)
            if node.right is None:
                node.right = new_node
                return
            else:
                queue.append(node.right)

# -----------------------------
# Árvore AVL
# -----------------------------
class AVLNode:
    def __init__(self, key):
        self.key = key #valor do nó
        self.left = None #subárvore esquerda
        self.right = None #subárvore direita
        self.height = 1 #altura do nó (usado para calculo de balanceamento)

class AVLTree:
    def __init__(self):
        self.root = None 
        self.rotations = 0 #counter de rotações

    def insert(self, root, key):
        if not root:
            return AVLNode(key)
        elif key == root.key:
            return root # não insere se já existir
        elif key < root.key: 
            root.left = self.insert(root.left, key) #esqueda se menor
        else:
            root.right = self.insert(root.right, key) #direita se maior
        
        # Atualiza a altura do nó atual e verifica o balanceamento     
        root.height = 1 + max(self.get_height(root.left), self.get_height(root.right))
        balance = self.get_balance(root)

        # Se o nó ficar desbalanceado:
        # Rotação simples à direita     
        if balance > 1 and key < root.left.key:
            self.rotations += 1
            return self.rotate_right(root)
        
        # Rotação simples à esquerda
        if balance < -1 and key > root.right.key:
            self.rotations += 1
            return self.rotate_left(root)
        
        # Rotação dupla à direita
        if balance > 1 and key > root.left.key:
            root.left = self.rotate_left(root.left)
            self.rotations += 2
            return self.rotate_right(root)
        
        # Rotação dupla à esquerda
        if balance < -1 and key < root.right.key:
            root.right = self.rotate_right(root.right)
            self.rotations += 2
            return self.rotate_left(root)

        return root

#obtem a altura do nó
    def get_height(self, node):
        return node.height if node else 0

    # obtém o balanceamento do nó
    def get_balance(self, node):
        return self.get_height(node.left) - self.get_height(node.right) if node else 0


    def rotate_left(self, z):
        y = z.right #arvore direita
        T2 = y.left #subárvore esquerda da direita
        y.left = z #rotaciona o nó z para a esquerda
        z.right = T2 #esquerda da direita = z
        # Atualiza as alturas
        z.height = 1 + max(self.get_height(z.left), self.get_height(z.right))
        y.height = 1 + max(self.get_height(y.left), self.get_height(y.right))
        return y

    def rotate_right(self, z):
        y = z.left
        T3 = y.right
        y.right = z
        z.left = T3
        z.height = 1 + max(self.get_height(z.left), self.get_height(z.right))
        y.height = 1 + max(self.get_height(y.left), self.get_height(y.right))
        return y

# -----------------------------
# Árvore Vermelho-Preto (VP)
# -----------------------------
RED = True
BLACK = False

class RBNode:
    def __init__(self, key, color=RED):
        self.key = key #valor
        self.left = None #filho esquerdo
        self.right = None #filho direito
        self.color = color #default= vermelho

class RBTree:
    def __init__(self):
        self.root = None
        self.rotations = 0 #counter de rotações

    def is_red(self, node):
        return node.color if node else BLACK #check se é vermelho se for nulo retorna preto

    def rotate_left(self, h):
        x = h.right
        h.right = x.left
        x.left = h
        x.color = h.color
        h.color = RED
        self.rotations += 1
        return x

    def rotate_right(self, h):
        x = h.left
        h.left = x.right
        x.right = h
        x.color = h.color
        h.color = RED
        self.rotations += 1
        return x

    def flip_colors(self, h):
        h.color = RED
        if h.left:
            h.left.color = BLACK
        if h.right:
            h.right.color = BLACK

    def insert(self, h, key):
        if not h:
            return RBNode(key)

        if key < h.key:
            h.left = self.insert(h.left, key)
        elif key > h.key:
            h.right = self.insert(h.right, key)
        else:
            return h

        if self.is_red(h.right) and not self.is_red(h.left):
            h = self.rotate_left(h)
        if self.is_red(h.left) and self.is_red(h.left.left):
            h = self.rotate_right(h)
        if self.is_red(h.left) and self.is_red(h.right):
            self.flip_colors(h)

        return h

    def insert_key(self, key):
        self.root = self.insert(self.root, key)
        self.root.color = BLACK

# -----------------------------
# Árvore Treap
# -----------------------------
class TreapNode:
    def __init__(self, key):
        self.key = key
        self.priority = random.randint(1, 100000)
        self.left = None
        self.right = None

class Treap:
    def __init__(self):
        self.root = None
        self.rotations = 0

    def rotate_right(self, y):
        x = y.left
        y.left = x.right
        x.right = y
        self.rotations += 1
        return x

    def rotate_left(self, x):
        y = x.right
        x.right = y.left
        y.left = x
        self.rotations += 1
        return y

    def insert(self, root, key):
        if root is None:
            return TreapNode(key)
        if key == root.key:
            return root
        elif key < root.key:
            root.left = self.insert(root.left, key)
            if root.left.priority > root.priority:
                root = self.rotate_right(root)
        else:
            root.right = self.insert(root.right, key)
            if root.right.priority > root.priority:
                root = self.rotate_left(root)
        return root

    def insert_key(self, key):
        self.root = self.insert(self.root, key)

# -----------------------------
# Geração e Testes
# -----------------------------
def gerar_conjuntos(n=10000):
    unique = random.sample(range(1, n * 10), n)
    poucos_repetidos = [random.choice(unique) for _ in range(n)]
    muitos_repetidos = [random.choice(unique[:int(n * 0.1)]) for _ in range(n)]
    return {
        'A': sorted(poucos_repetidos),
        'B': sorted(poucos_repetidos, reverse=True),
        'C': random.sample(poucos_repetidos, len(poucos_repetidos)),
        'D': random.sample(muitos_repetidos, len(muitos_repetidos))
    }

def testar_insercao(conjuntos):
    resultados = {'Binária': {},'AVL': {}, 'VP': {}, 'Treap': {}}
    for nome, dados in conjuntos.items():
        '''
        # Binária
        tree = binTree()
        inicio = time.time()
        for x in dados:
            tree.insert(x)
        fim = time.time()
        resultados['Binária'][nome] = (fim - inicio, 0)
        '''
        # AVL
        print(f"Testando o conjunto {nome} com a árvore AVL...")
        avl = AVLTree()
        inicio = time.time()
        for x in dados:
            avl.root = avl.insert(avl.root, x)
        fim = time.time()
        resultados['AVL'][nome] = (fim - inicio, avl.rotations)

        # VP
        print(f"Testando o conjunto {nome} com a árvore VP...")
        rb = RBTree()
        inicio = time.time()
        for x in dados:
            rb.insert_key(x)
        fim = time.time()
        resultados['VP'][nome] = (fim - inicio, rb.rotations)

        # Treap
        print(f"Testando o conjunto {nome} com a TREAP...")
        treap = Treap()
        inicio = time.time()
        for x in dados:
            treap.insert_key(x)
        fim = time.time()
        resultados['Treap'][nome] = (fim - inicio, treap.rotations)
    return resultados

def imprimir_resultados(resultados):
    for tipo, dados in resultados.items():
        print(f"\nEstrutura: {tipo}")
        for conjunto, (tempo, rotacoes) in dados.items():
            print(f"  Conjunto {conjunto}: Tempo = {tempo:.4f}s | Rotações = {rotacoes}")

# -----------------------------
# Execução Principal
# -----------------------------
if __name__ == "__main__":
    print("Conjuntos de 1*10^6 elementos")
    conjuntos = gerar_conjuntos(1000000)
    resultados = testar_insercao(conjuntos)
    imprimir_resultados(resultados)
    
   
    
   