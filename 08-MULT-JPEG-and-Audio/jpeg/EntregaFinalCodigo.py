import numpy as np
import matplotlib.colors as clr
import matplotlib.pyplot as plt
from scipy.fft import dct, idct
import cv2

#color maps
cmRed=clr.LinearSegmentedColormap.from_list('red', ['black', 'red'],N=256)
cmGreen=clr.LinearSegmentedColormap.from_list('green', ['black', 'green'],N=256)
cmBlue=clr.LinearSegmentedColormap.from_list('blue', ['black', 'blue'],N=256)
cmGray=clr.LinearSegmentedColormap.from_list('gray', ['black', 'white'],N=256)
cmCr=clr.LinearSegmentedColormap.from_list('Cr', ['black', 'red'],N=256)
cmCy=clr.LinearSegmentedColormap.from_list('Cy', ['black', 'cyan'],N=256)

#-----Funcoes que mostram a imagem-----#

#Mostra a imagem
def showImage(img, title, cmap=None, axis='off'):

    plt.figure()
    plt.imshow(img, cmap=cmap)
    plt.axis(axis)
    plt.title(title)
    plt.show()

    return

#Mostra imagem apos fazer DCT
def showDCT(img, title, cmap=None, axis='off'):

    plt.figure()
    plt.imshow(np.log(abs(img) + 0.0001), cmap=cmap)
    plt.axis(axis)
    plt.title(title)
    plt.show()
    
    return

#-----Padding e Depadding-----#

def padding(cor):
    data_type = cor.dtype
    l, c = cor.shape

    #Adiciona linhas de zeros
    ad = 32 - l%32
    pad = np.full((ad, c), cor[l-1][:])
    cor = np.vstack((cor, pad))
    
    l, c = cor.shape

    #Adiciona colunas de zeros
    ad = 32 - c%32
    pad = np.full((ad, l), cor[:, c-1])
    cor = np.hstack((cor, pad.T))

    return cor

#Retira o padding de um canal, tendo a informação do seu tamanho original
def depadding(cor, tam):
    #Seleciona valores usados na imagens original (ou seja, sem padding)
    res = cor[:tam[0],:tam[1]]
    return res

#-----RGB to YCbCr e YCbCr to RGB-----#

matConversao = np.array([[0.299, 0.587, 0.114], [-0.168736, -0.331264, 0.5], [0.5, -0.418688, -0.081312]])

#Converte imagem do modelo de cor RGB para o modelo de cor YCbCr
def RGBtoYCbCr(r,g,b):

    img = np.stack((r, g, b), axis = 2)
    res = np.dot(img, matConversao.T)
    
    return res[:,:,0], res[:,:,1] + 128, res[:,:,2] + 128

#Converte imagem do modelo de cor YCbCr para o modelo de cor RGB
def YCbCrToRGB(Y, Cb, Cr):

    #Calcula a matriz de conversão de YCbCr para RGB
    transInv = np.linalg.inv(matConversao)

    #Outra forma de mudar o modelo de cor. Melhor?
    img = np.stack((Y, Cb-128, Cr-128), axis = 2)
    res = np.dot(img, transInv.T)

    res = np.round(res).clip(0, 255).astype(np.uint8)

    return res[:, :, 0], res[:, :, 1], res[:, :, 2]

#-----Downsampling e Upsampling-----#

#Faz downsampling às componentes Cb e Cr, tendo a, b, c como fatores
def downsampling(Y, Cb, Cr, a:int, b:int, c:int, inter: int):
    inter = inter%5
    interpolation_cases = [cv2.INTER_LINEAR, cv2.INTER_NEAREST, cv2.INTER_CUBIC, cv2.INTER_LANCZOS4, cv2.INTER_AREA]
    Y_d = Y

    #Só aceitar fatores que façam sentido
    aceita = [1, 2, 4]
    if (a != 4 or b not in aceita or (c not in aceita and c != 0)):
        print("Ratio errado")
        return Y, Cb, Cr 

    #Caso seja sub-amostragem horizontal e vertical
    if (c == 0):
        Cb_d = cv2.resize(Cb, None, fx = b/a, fy = b/a, interpolation=interpolation_cases[inter])
        Cr_d = cv2.resize(Cr, None, fx = b/a, fy = b/a, interpolation=interpolation_cases[inter])
    #Caso seja apenas sub-amostragem horizontal
    else:
        Cr_d = cv2.resize(Cr, None, fx = b/a, fy = 1, interpolation=interpolation_cases[inter])
        Cb_d = cv2.resize(Cb, None, fx = c/a, fy = 1, interpolation=interpolation_cases[inter])

    return Y_d, Cb_d, Cr_d

#Faz upsampling às componentes Cb_d e Cr_d, tendo tamPosPadding como tamanho original de Cb e Cr (padded)
def upsampling(Y_d, Cb_d, Cr_d, a:int, b:int, c:int, inter: int):
    #Usa interpolation para fazer upsampling
    inter = inter%5
    interpolation_cases = [cv2.INTER_LINEAR, cv2.INTER_NEAREST, cv2.INTER_CUBIC, cv2.INTER_LANCZOS4, cv2.INTER_AREA]
    Y = Y_d

    #Só aceitar fatores que façam sentido
    aceita = [1, 2, 4]
    if (a != 4 or b not in aceita or (c not in aceita and c != 0)):
        print("Ratio errado")
        return Y_d, Cb_d, Cr_d 

    #Caso seja sub-amostragem horizontal e vertical
    if (c == 0):
        Cb = cv2.resize(Cb_d, None, fx = a/b, fy = a/b, interpolation=interpolation_cases[inter])
        Cr = cv2.resize(Cr_d, None, fx = a/b, fy = a/b, interpolation=interpolation_cases[inter])
    #Caso seja apenas sub-amostragem horizontal
    else:
        Cr = cv2.resize(Cr_d, None, fx = a/b, fy = 1, interpolation=interpolation_cases[inter])
        Cb = cv2.resize(Cb_d, None, fx = a/c, fy = 1, interpolation=interpolation_cases[inter])
    return Y, Cb, Cr

#-----DCT e IDCT-----#

#Faz a transformada de Coseno Discreta, dependendo do tamanho das sub-janelas. Se o tamanho de alguma destas for 0 ou não for um divisor de um dos eixos das componentes, faz DCT de toda a imagem
def DCT(Y_d, Cb_d, Cr_d, janTamX: int, janTamY: int):
    #De onde fui buscar a ideia
    #https://stackoverflow.com/questions/35534626/how-to-apply-functions-to-numpy-8-8-blocks

    #Verifica se o tamanho da janela é válido. Se não for, faz DCT para toda a imagem
    if (janTamX == 0 or janTamY == 0 or Y_d.shape[0]%janTamX != 0 or Y_d.shape[1]%janTamY != 0 or Cb_d.shape[0]%janTamX != 0 or Cb_d.shape[1]%janTamY != 0 or Cr_d.shape[0]%janTamX != 0 or Cr_d.shape[1]%janTamY != 0):
        print("Tamanho da subjanela inválido")
        Y_dct = dct(dct(Y_d, norm="ortho").T, norm="ortho").T
        Cb_dct = dct(dct(Cb_d, norm="ortho").T, norm="ortho").T
        Cr_dct = dct(dct(Cr_d, norm="ortho").T, norm="ortho").T
    else:
        #Faz reshape de (l, c) para (l//janTamX, janTamX, c//janTamY, janTamY), para fazer a DCT por janelas
        Y_d_aux = np.reshape(Y_d, (Y_d.shape[0]//janTamX, janTamX, Y_d.shape[1]//janTamY, janTamY))
        Cb_d_aux = np.reshape(Cb_d, (Cb_d.shape[0]//janTamX, janTamX, Cb_d.shape[1]//janTamY, janTamY))
        Cr_d_aux = np.reshape(Cr_d, (Cr_d.shape[0]//janTamX, janTamX, Cr_d.shape[1]//janTamY, janTamY))

        #Calcula DCT só para as janelas (submatrizes janTamX*janTamY, eixos 1 e 3)
        Y_dct_aux = dct(dct(Y_d_aux, norm="ortho", axis=1), norm="ortho", axis=3) 
        Cb_dct_aux = dct(dct(Cb_d_aux, norm="ortho", axis=1), norm="ortho", axis=3) 
        Cr_dct_aux = dct(dct(Cr_d_aux, norm="ortho", axis=1), norm="ortho", axis=3) 

        #Coloca as componentes da imagem no seu formato original, de (l//janTamX, janTamX, c//janTamY, janTamY) para (l, c)
        Y_dct = np.reshape(Y_dct_aux, (Y_d.shape[0], Y_d.shape[1]))
        Cb_dct = np.reshape(Cb_dct_aux, (Cb_d.shape[0], Cb_d.shape[1]))
        Cr_dct = np.reshape(Cr_dct_aux, (Cr_d.shape[0], Cr_d.shape[1]))
        
    return Y_dct, Cb_dct, Cr_dct

#Faz a inversa da Transformada de Coseno Discreta, dependendo do tamanho das sub-janelas. De forma semelhante à DCT, se o tamanho de alguma destas for 0 ou não for um divisor de um dos eixos das componentes, faz IDCT de toda a imagem
def IDCT(Y_dct, Cb_dct, Cr_dct, janTamX:int, janTamY:int):

    #Verifica se o tamanho da janela é válido. Se não for, faz DCT para toda a imagem
    if (janTamX == 0 or janTamY == 0 or Y_dct.shape[0]%janTamX != 0 or Y_dct.shape[1]%janTamY != 0 or Cb_dct.shape[0]%janTamX != 0 or Cb_dct.shape[1]%janTamY != 0 or Cr_dct.shape[0]%janTamX != 0 or Cr_dct.shape[1]%janTamY != 0):
        Y_d = idct(idct(Y_dct, norm="ortho").T, norm="ortho").T
        Cb_d = idct(idct(Cb_dct, norm="ortho").T, norm="ortho").T
        Cr_d = idct(idct(Cr_dct, norm="ortho").T, norm="ortho").T
    else:
        #Faz reshape de (l, c) para (l//janTamX, janTamX, c//janTamY, janTamY), para fazer a IDCT por janelas
        Y_dct_aux = Y_dct.reshape(Y_dct.shape[0]//janTamX, janTamX, Y_dct.shape[1]//janTamY, janTamY)
        Cb_dct_aux = Cb_dct.reshape(Cb_dct.shape[0]//janTamX, janTamX, Cb_dct.shape[1]//janTamY, janTamY)
        Cr_dct_aux = Cr_dct.reshape(Cr_dct.shape[0]//janTamX, janTamX, Cr_dct.shape[1]//janTamY, janTamY)

        #Calcula DCT só para as janelas (submatrizes janTamX*janTamY, eixos 1 e 3)
        Y_d_aux = idct(idct(Y_dct_aux, norm="ortho", axis=3), norm="ortho",axis=1)
        Cb_d_aux = idct(idct(Cb_dct_aux, norm="ortho", axis=3), norm="ortho", axis=1)
        Cr_d_aux = idct(idct(Cr_dct_aux, norm="ortho", axis=3), norm="ortho", axis=1)

        #Coloca as componentes da imagem no seu formato original, de (l//janTamX, janTamX, c//janTamY, janTamY) para (l, c)
        Y_d = Y_d_aux.reshape(Y_dct.shape[0], Y_dct.shape[1])
        Cb_d = Cb_d_aux.reshape(Cb_dct.shape[0], Cb_dct.shape[1])
        Cr_d = Cr_d_aux.reshape(Cr_dct.shape[0], Cr_dct.shape[1])

    return Y_d, Cb_d, Cr_d

#-----Quantization e Dequantization-----#

Q_Y = np.array([[16, 11, 10, 16, 24, 40, 51, 61],
                [12, 12, 14, 19, 26, 58, 60, 55],
                [14, 13, 16, 24, 40, 57, 69, 56],
                [14, 17, 22, 29, 51, 87, 80, 62],
                [18, 22, 37, 56, 68, 109, 103, 77],
                [24, 35, 55, 64, 81, 104, 113, 92],
                [49, 64, 78, 87, 103, 121, 120, 100],
                [72, 92, 95, 98, 112, 100, 103, 99]]
               )
Q_CbCr = np.array([[17, 18, 24, 47, 99, 99, 99, 99],
                    [18, 21, 26, 66, 99, 99, 99, 99],
                    [24, 26, 56, 99, 99, 99, 99, 99],
                    [47, 66, 99, 99, 99, 99, 99, 99],
                    [99, 99, 99, 99, 99, 99, 99, 99],
                    [99, 99, 99, 99, 99, 99, 99, 99],
                    [99, 99, 99, 99, 99, 99, 99, 99],
                    [99, 99, 99, 99, 99, 99, 99, 99]]
                    )

#Quantiza os valores da dct. Altera a Q_Y e a Q_CbCr segundo o quality_factor. O resultado de Q_Y (QsY) é dividido pela Y_dct, em blocos de 8*8, e o resultado de Q_CbCr (QsCbCr) é dividido pelas Cb_dct e Cr_dct, em blocos de 8*8, respetivamente
def quantize(Y_dct, Cb_dct, Cr_dct, quality_factor):
    # Fator de escala depende do fator de qualidade
    if quality_factor < 50:
        scale = 50 / quality_factor
    else:
        scale = (100 - quality_factor) / 50

    if (scale > 0):
        QsY = np.round(Q_Y * scale).clip(1, 255).astype(np.uint8)
        QsCbCr = np.round(Q_CbCr * scale).clip(1, 255).astype(np.uint8)
    else:
        QsY = np.ones([8, 8]).astype(np.uint8)
        QsCbCr = np.ones([8, 8]).astype(np.uint8)

    #Cria matrizes aumentadas (tiled) a partir de QsY e QsCbCr, com os mesmos tamanhos de Y_dct, Cb_dct e Cr_dct, para poder fazer a divisão
    augQsY = np.tile(QsY, (Y_dct.shape[0]//8, Y_dct.shape[1]//8))
    augQsCb = np.tile(QsCbCr, (Cb_dct.shape[0]//8, Cb_dct.shape[1]//8))
    augQsCr = np.tile(QsCbCr, (Cr_dct.shape[0]//8, Cr_dct.shape[1]//8))

    print("QY")
    print(augQsY)
    
    #Faz a divisão entre as dct's e as respetivas matrizes de quantização auxiliares
    Y_Q = np.round(Y_dct/augQsY).astype(np.int32)
    Cb_Q = np.round(Cb_dct/augQsCb).astype(np.int32)
    Cr_Q = np.round(Cr_dct/augQsCr).astype(np.int32)

    return Y_Q, Cb_Q, Cr_Q

#Faz a desquantização. Altera a Q_Y e a Q_CbCr segundo o quality_factor. O resultado de Q_Y (QsY) é multiplicado pela Y_Q, em blocos de 8*8, e o resultado de Q_CbCr (QsCbCr) é multiplicado pelas Cb_Q e Cr_Q, em blocos 8*8, respetivamente
def dequantize(Y_Q, Cb_Q, Cr_Q, quality_factor):
    # Fator de escala depende do fator de qualidade
    if quality_factor < 50:
        scale = 50 / quality_factor
    else:
        scale = (100 - quality_factor) / 50

    if (scale > 0):
        QsY = np.round(Q_Y * scale).clip(1, 255).astype(np.uint8)
        QsCbCr = np.round(Q_CbCr * scale).clip(1, 255).astype(np.uint8)
    else:
        QsY = np.ones([8, 8]).astype(np.uint8)
        QsCbCr = np.ones([8, 8]).astype(np.uint8)

    #Cria matrizes aumentadas (tiled) a partir de QsY e QsCbCr, com os mesmos tamanhos de Y_Q, Cb_Q e Cr_Q, para poder fazer a divisão
    augQsY = np.tile(QsY, (Y_Q.shape[0]//8, Y_Q.shape[1]//8))
    augQsCb = np.tile(QsCbCr, (Cb_Q.shape[0]//8, Cb_Q.shape[1]//8))
    augQsCr = np.tile(QsCbCr, (Cr_Q.shape[0]//8, Cr_Q.shape[1]//8))

    print("QY")
    print(augQsY)
    
    #Faz a multiplicação entre as matrizes quantizadas e as matrizes de quantização auxiliares
    Y_dct = Y_Q * augQsY
    Cb_dct = Cb_Q * augQsCb
    Cr_dct = Cr_Q * augQsCr

    return Y_dct, Cb_dct, Cr_dct

#-----DPCM e IDPCM-----#

#Faz Differential pulse-code modulation para todos os quadrados 8*8 da matriz. Cada casa de cada matrix 8*8 é comparada com a sua correspondente nas outras matrizes 8*8. O cálculo é feito reposicionando-as num array de matriz 8*8, como é feito na linguagem C
def DPCM(Y_Q, Cb_Q, Cr_Q):
    #Para cada canal:

    #Cria uma matriz em que cada "casa" é uma matriz 8*8, mantendo a posição relativa de cada uma destas matrizes
    Y_Q_aux = np.reshape(Y_Q, (Y_Q.shape[0]//8, 8, Y_Q.shape[1]//8, 8))
    Cb_Q_aux = np.reshape(Cb_Q, (Cb_Q.shape[0]//8, 8, Cb_Q.shape[1]//8, 8))
    Cr_Q_aux = np.reshape(Cr_Q, (Cr_Q.shape[0]//8, 8, Cr_Q.shape[1]//8, 8))

    #Transpõe a matriz resultante para uma melhor posição
    Y_Q_aux = np.transpose(Y_Q_aux, (2, 0, 1, 3))
    Cb_Q_aux = np.transpose(Cb_Q_aux, (2, 0, 1, 3))
    Cr_Q_aux = np.transpose(Cr_Q_aux, (2, 0, 1, 3))

    #Faz um "flatten" dos 2 primeiros eixos da matriz resultante, tornando esta matriz num "array" de matrizes 8*8
    #Y_Q_aux[0] corresponde a Y_Q[0:8, 0:8] e Y_Q_aux[Y_Q.shape[1]//8] corresponde a Y_Q[8:16, 0:8]
    Y_Q_aux = np.reshape(Y_Q_aux, ((Y_Q.shape[0]*Y_Q.shape[1])//64, 8, 8), order = 'F')
    Cb_Q_aux = np.reshape(Cb_Q_aux, ((Cb_Q.shape[0]*Cb_Q.shape[1])//64, 8, 8), order = 'F')
    Cr_Q_aux = np.reshape(Cr_Q_aux, ((Cr_Q.shape[0]*Cr_Q.shape[1])//64, 8, 8), order = 'F')

    #Calcula a DIFF para este "array"
    Y_Q_aux[1:, 0, 0] = Y_Q_aux[1:, 0, 0] - Y_Q_aux[:-1, 0, 0]
    Cb_Q_aux[1:, 0, 0] = Cb_Q_aux[1:, 0, 0] - Cb_Q_aux[:-1, 0, 0]
    Cr_Q_aux[1:, 0, 0] = Cr_Q_aux[1:, 0, 0] - Cr_Q_aux[:-1, 0, 0]

    #Reconstroi a matriz DIFF para o formato original, sem perder a posição relativa de nenhum valor
    Y_dpcm_aux = np.reshape(Y_Q_aux, (Y_Q.shape[1]//8, Y_Q.shape[0]//8, 8, 8), order = 'F')
    Cb_dpcm_aux = np.reshape(Cb_Q_aux, (Cb_Q.shape[1]//8, Cb_Q.shape[0]//8, 8, 8), order = 'F')
    Cr_dpcm_aux = np.reshape(Cr_Q_aux, (Cr_Q.shape[1]//8, Cr_Q.shape[0]//8, 8, 8), order = 'F')

    Y_dpcm_aux = np.transpose(Y_dpcm_aux, (1, 2, 0, 3))
    Cb_dpcm_aux = np.transpose(Cb_dpcm_aux, (1, 2, 0, 3))
    Cr_dpcm_aux = np.transpose(Cr_dpcm_aux, (1, 2, 0, 3))

    Y_dpcm = np.reshape(Y_dpcm_aux, (Y_Q.shape[0], Y_Q.shape[1]))
    Cb_dpcm = np.reshape(Cb_dpcm_aux, (Cb_Q.shape[0], Cb_Q.shape[1]))
    Cr_dpcm = np.reshape(Cr_dpcm_aux, (Cr_Q.shape[0], Cr_Q.shape[1]))

    return Y_dpcm, Cb_dpcm, Cr_dpcm

#Faz o inverso da Differential pulse-code modulation para todos os quadrados 8*8 da matriz. Cada casa de cada matrix 8*8 é comparada com a sua correspondente nas outras matrizes 8*8. O cálculo é feito reposicionando-as num array de matriz 8*8, como é feito na linguagem C
def IDPCM(Y_dpcm, Cb_dpcm, Cr_dpcm):
    #Para cada canal:

    #Cria uma matriz em que cada "casa" é uma matriz 8*8, mantendo a posição relativa de cada uma destas matrizes
    Y_dpcm_aux = np.reshape(Y_dpcm, (Y_dpcm.shape[0]//8, 8, Y_dpcm.shape[1]//8, 8))
    Cb_dpcm_aux = np.reshape(Cb_dpcm, (Cb_dpcm.shape[0]//8, 8, Cb_dpcm.shape[1]//8, 8))
    Cr_dpcm_aux = np.reshape(Cr_dpcm, (Cr_dpcm.shape[0]//8, 8, Cr_dpcm.shape[1]//8, 8))

    #Transpõe a matriz resultante para uma melhor posição
    Y_dpcm_aux = np.transpose(Y_dpcm_aux, (2, 0, 1, 3))
    Cb_dpcm_aux = np.transpose(Cb_dpcm_aux, (2, 0, 1, 3))
    Cr_dpcm_aux = np.transpose(Cr_dpcm_aux, (2, 0, 1, 3))

    #Faz um "flatten" dos 2 primeiros eixos da matriz resultante, tornando esta matriz num "array" de matrizes 8*8
    #Y_dpcm_aux[0] corresponde a Y_dpcm[0:8, 0:8] e Y_dpcm_aux[Y_dpcm.shape[1]//8] corresponde a Y_dpcm[8:16, 0:8]
    Y_dpcm_aux = np.reshape(Y_dpcm_aux, ((Y_dpcm.shape[0]*Y_dpcm.shape[1])//64, 8, 8), order = 'F')
    Cb_dpcm_aux = np.reshape(Cb_dpcm_aux, ((Cb_dpcm.shape[0]*Cb_dpcm.shape[1])//64, 8, 8), order = 'F')
    Cr_dpcm_aux = np.reshape(Cr_dpcm_aux, ((Cr_dpcm.shape[0]*Cr_dpcm.shape[1])//64, 8, 8), order = 'F')

    #Calcula a matriz Q para este "array"
    Y_dpcm_aux[:, 0, 0] = np.cumsum(Y_dpcm_aux[:, 0, 0])
    Cb_dpcm_aux[:, 0, 0] = np.cumsum(Cb_dpcm_aux[:, 0, 0])
    Cr_dpcm_aux[:, 0, 0] = np.cumsum(Cr_dpcm_aux[:, 0, 0])

    #Reconstroi a matriz Q para o formato original, sem perder a posição relativa de nenhum valor
    Y_Q_aux = np.reshape(Y_dpcm_aux, (Y_dpcm.shape[1]//8, Y_dpcm.shape[0]//8, 8, 8), order = 'F')
    Cb_Q_aux = np.reshape(Cb_dpcm_aux, (Cb_dpcm.shape[1]//8, Cb_dpcm.shape[0]//8, 8, 8), order = 'F')
    Cr_Q_aux = np.reshape(Cr_dpcm_aux, (Cr_dpcm.shape[1]//8, Cr_dpcm.shape[0]//8, 8, 8), order = 'F')

    Y_Q_aux = np.transpose(Y_Q_aux, (1, 2, 0, 3))
    Cb_Q_aux = np.transpose(Cb_Q_aux, (1, 2, 0, 3))
    Cr_Q_aux = np.transpose(Cr_Q_aux, (1, 2, 0, 3))

    Y_Q = np.reshape(Y_Q_aux, (Y_dpcm.shape[0], Y_dpcm.shape[1]))
    Cb_Q = np.reshape(Cb_Q_aux, (Cb_dpcm.shape[0], Cb_dpcm.shape[1]))
    Cr_Q = np.reshape(Cr_Q_aux, (Cr_dpcm.shape[0], Cr_dpcm.shape[1]))

    return Y_Q, Cb_Q, Cr_Q

#-----Metricas-----#

def metricas(original,compacted):
    
    R_Original=original[:,:,0]
    G_Original=original[:,:,1]
    B_Original=original[:,:,2]
    R_Compacted=compacted[:,:,0]
    G_Compacted=compacted[:,:,1]    
    B_Compacted=compacted[:,:,2]

    Y_Original, _, _ =RGBtoYCbCr(R_Original,G_Original,B_Original)
    Y_Compacted, _, _ =RGBtoYCbCr(R_Compacted,G_Compacted,B_Compacted)

    Y_Difference=np.abs(np.int32(Y_Original) - np.int32(Y_Compacted))
    diff = np.abs(np.int32(original) - np.int32(compacted))

    showImage(Y_Difference,'Imagem diferenças',cmap=cmGray)
    
    # Cálculo Mean Squared Error
    mse = np.mean(np.square(np.abs(diff)))*3
    
    #Calculo RootMeanSquareError (RMSE) 
    rmse = np.sqrt(mse)
    
    #Calcculo signal to noise ratio (SNR)
    sinal_power = np.mean(np.square(np.int32(original)))
    if mse > 0: # para evitar divisão por zero
          snr = 10 * np.log10(sinal_power / mse)
    else:
          snr = np.inf #se noise_power=0, então snr é infinito ou seja qualidade é perfeita
          
    #Calculo Peak Signal to Noise Ratio (PSNR)
    max_pixel = np.max(np.int32(original))
    if mse>0:
        psnr = 10 * np.log10((max_pixel**2) / mse)
    else:
        psnr = np.inf
        
    #calcula a diferença max e media 
    max_diff = np.max(Y_Difference)
    avg_diff = np.mean(Y_Difference)
    
    return mse, rmse, snr, psnr, max_diff, avg_diff

#-----Testas melhor interpolacao-----#

def testBestInterpolation(img, ratio: tuple[int, int, int]):
    R=img[:,:,0]
    G=img[:,:,1]
    B=img[:,:,2]

    tamOriginal = R.shape

    R = padding(R)
    G = padding(G)
    B = padding(B)

    tamPosPadding = R.shape

    Y, Cb, Cr =RGBtoYCbCr(R,G,B)
    data_type = Y.dtype
    img = np.zeros((tamPosPadding[0],tamPosPadding[1],3),dtype=data_type)
    img[:,:,0] = Y
    img[:,:,1] = Cb
    img[:,:,2] = Cr

    menorDesvioPadrao = 1000000
    melhorInterpolacao = 0
    desvioPadrao = []
    for i in range(5):
        Y_d, Cb_d, Cr_d = downsampling(Y, Cb, Cr, ratio[0], ratio[1], ratio[2], i)
        Y_rec, Cb_rec, Cr_rec = upsampling(Y_d, Cb_d, Cr_d, ratio[0], ratio[1], ratio[2], i)
        data_type = Y_rec.dtype
        imgRec = np.zeros((tamPosPadding[0],tamPosPadding[1],3),dtype=data_type)
        imgRec[:,:,0] = Y_rec
        imgRec[:,:,1] = Cb_rec
        imgRec[:,:,2] = Cr_rec
        aux = np.std(np.int32(img) - np.int32(imgRec))
        desvioPadrao.append(aux)
        if (aux < menorDesvioPadrao):
            menorDesvioPadrao = aux
            melhorInterpolacao = i

    return melhorInterpolacao, tuple(desvioPadrao)

#Separa a imagem nas suas componentes RGB, acrescenta padding e muda o modelo de cor para YCbCr. De seguida, faz o downsampling das suas componentes e aplica DCT em janelas ou na imagem completa
#Retorna as componentes Y, Cb, Cr e também a shape original das componentes
def encoder(img, ratio: tuple[int, int, int], interpolation_cases, BSx, BSy, quality_factor):

    #Separa a imagem nas suas componentes RGB
    R=img[:,:,0]
    G=img[:,:,1]
    B=img[:,:,2]

    showImage(R, "R", cmap=cmRed)
    showImage(G, "G", cmap=cmGreen)
    showImage(B, "B", cmap=cmBlue)

    print("R[8:16, 8:16]")
    print(R[8:16, 8:16])

    #Calcula shape de uma componente (é igual para todas)
    tamOriginal = R.shape

    #Aplica padding
    R = padding(R)
    G = padding(G)
    B = padding(B)

    #print(R[8:16, 8:16])

    tamPosPadding = R.shape

    #Aplica mudança de modelo de cor para YCbCr
    Y, Cb, Cr =RGBtoYCbCr(R,G,B)
    showImage(Y, "Y", cmap=cmGray)
    showImage(Cb, "Cb", cmap=cmGray)
    showImage(Cr, "Cr", cmap=cmGray)
    print("Y[8:16,8:16]\n",Y[8:16,8:16])
    print("Cb[8:16,8:16]\n",Cb[8:16,8:16])

    #Aplica os parâmetros de downsampling à componentes da imagem
    Y_d, Cb_d, Cr_d = downsampling(Y, Cb, Cr, ratio[0], ratio[1], ratio[2], interpolation_cases)
    showImage(Y_d, "Y downsampling " + str(ratio[0]) + ":" + str(ratio[1]) + ":" + str(ratio[2]), cmap=cmGray)
    showImage(Cb_d, "Cb downsampling " + str(ratio[0]) + ":" + str(ratio[1]) + ":" + str(ratio[2]), cmap=cmGray)
    showImage(Cr_d, "Cr downsampling " + str(ratio[0]) + ":" + str(ratio[1]) + ":" + str(ratio[2]), cmap=cmGray)
    print("Cb[8:16, 8:16]")
    print(Cb[8:16, 8:16])

    Y_dct, Cb_dct, Cr_dct = DCT(Y_d, Cb_d, Cr_d, 0, 0)
    showDCT(Y_dct, "Y_DCT", cmap=cmGray)
    showDCT(Cb_dct, "Cb_DCT", cmap=cmGray)
    showDCT(Cr_dct, "Cr_DCT", cmap=cmGray)

    #Aplica a Transformada de Coseno Discreta às componentes downsampled
    Yb_dct, Cbb_dct, Crb_dct = DCT(Y_d, Cb_d, Cr_d, BSx, BSy)
    showDCT(Yb_dct, "Yb_DCT", cmap=cmGray)
    showDCT(Cbb_dct, "Cbb_dct", cmap=cmGray)
    showDCT(Crb_dct, "Crb_dct", cmap=cmGray)
    print("Yb_DCT" + str(BSx) + "*" + str(BSy) + "[8:16, 8:16]")
    print(Yb_dct[8:16, 8:16])

    Y_Q, Cb_Q, Cr_Q = quantize(Yb_dct, Cbb_dct, Crb_dct, quality_factor)
    showDCT(Y_Q, "Yb_Q", cmap=cmGray)
    showDCT(Cb_Q, "Cbb_Q", cmap=cmGray)
    showDCT(Cr_Q, "Crb_Q", cmap=cmGray) 
    print("Yb_Q[8:16, 8:16]")
    print(Y_Q[8:16, 8:16])

    Y_dpcm, Cb_dpcm, Cr_dpcm = DPCM(Y_Q, Cb_Q, Cr_Q)
    showDCT(Y_dpcm, "Yb_DPCM", cmap=cmGray)
    showDCT(Cb_dpcm, "Cbb_DPCM", cmap=cmGray)
    showDCT(Cr_dpcm, "Crb_DPCM", cmap=cmGray)
    print("Yb_DPCM[8:16, 8:16]")
    print(Y_dpcm[8:16, 8:16])

    return Y_dpcm,Cb_dpcm,Cr_dpcm, tamOriginal

#Muda o modelo de cor, de RGB para YCbCr, e junta as componentes RGB, tirando antes o padding  (inverso de encoder)
#Retorna a imagem
def decoder(Y_dpcm, Cb_dpcm, Cr_dpcm, tamOriginal, ratio: tuple[int, int, int], interpolation_cases, BSx, BSy, quality_factor):
    showDCT(Y_dpcm, "Yb_iDPCM", cmap=cmGray)
    showDCT(Cb_dpcm, "Cbb_iDPCM", cmap=cmGray)
    showDCT(Cr_dpcm, "Crb_iDPCM", cmap=cmGray)
    print("Yb_iDPCM[8:16, 8:16]")
    print(Y_dpcm[8:16, 8:16])

    Y_Q, Cb_Q, Cr_Q = IDPCM(Y_dpcm, Cb_dpcm, Cr_dpcm)
    showDCT(Y_Q, "Yb_iQ", cmap=cmGray)
    showDCT(Cb_Q, "Cbb_iQ", cmap=cmGray)
    showDCT(Cr_Q, "Crb_iQ", cmap=cmGray)
    print("Yb_iQ[8:16, 8:16]")
    print(Y_Q[8:16, 8:16])

    Y_dct, Cb_dct, Cr_dct = dequantize(Y_Q, Cb_Q, Cr_Q, quality_factor)
    showDCT(Y_dct, "Yb_iDCT", cmap=cmGray)
    showDCT(Cb_dct, "Cbb_iDCT", cmap=cmGray)
    showDCT(Cr_dct, "Crb_iDCT", cmap=cmGray)
    print("Yb_iDCT[8:16, 8:16]")
    print(Y_dct[8:16, 8:16])

    #Aplica o inverso da Tranformada de Coseno Discreta
    Y_d, Cb_d, Cr_d = IDCT(Y_dct, Cb_dct, Cr_dct, BSx, BSy)

    #Aplica upsampling
    Y, Cb, Cr = upsampling(Y_d, Cb_d, Cr_d, ratio[0], ratio[1], ratio[2], interpolation_cases)
    showImage(Y, "Y upsampling", cmap=cmGray)
    showImage(Cb, "Cb upsampling", cmap=cmGray)
    showImage(Cr, "Cr upsampling", cmap=cmGray)
    print("Cb_UP[8:16, 8:16]")
    print(Cb[8:16, 8:16])

    #Aplica mudança de modelo de cor para RGB
    R, G, B = YCbCrToRGB(Y, Cb, Cr)

    #Retira padding a cada componente
    R = depadding(R, tamOriginal)
    G = depadding(G, tamOriginal)
    B = depadding(B, tamOriginal)

    print("R_decoded[8:16, 8:16]")
    print(R[8:16, 8:16])

    #Junta as componentes numa só imagem
    data_type = R.dtype
    imgRec = np.zeros((tamOriginal[0],tamOriginal[1],3),dtype=data_type)
    imgRec[:,:,0] = R
    imgRec[:,:,1] = G
    imgRec[:,:,2] = B

    return imgRec

def main():
    print ("Starting...")
    filename = 'airport.bmp'
    title = 'Imagem Original'
    img = plt.imread(filename)
    showImage(img,title)

    BSx, BSy = (8, 8)
    ratio = (4, 2, 2)
    interpolation_cases, desvioPadrao = testBestInterpolation(img, ratio)
    #print("Linear:", desvioPadrao[0])
    #print("Nearest:", desvioPadrao[1])
    #print("Cubic:", desvioPadrao[2])
    #print("LanczoS4:", desvioPadrao[3])
    #print("Area:", desvioPadrao[4])
    #print(interpolation_cases)
    qf = 75
    Y_dpcm, Cb_dpcm, Cr_dpcm, tamOriginal = encoder(img, ratio, interpolation_cases, BSx, BSy, qf)

    imgRec = decoder(Y_dpcm,Cb_dpcm,Cr_dpcm,tamOriginal, ratio, interpolation_cases, BSx, BSy, qf)

    title = 'Imagem Reconstruida'
    showImage(imgRec,title)

    mse, rmse, snr, psnr, max_diff, avg_diff = metricas(img, imgRec)
    print("MSE:", mse)
    print("RMSE:", rmse)
    print("SNR:", snr)
    print("PSNR:", psnr)
    print("Max diff:", max_diff)
    print("Avg diff:", avg_diff)
    
    print ("Done.")
    return

if __name__ == "__main__":
    main()
