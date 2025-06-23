import pygame
import math
import random

WIDTH, HEIGHT = 240, 240
GRAVITY = 9.81
h, velAviao = 1000, 55 


def CalcHorPos(speed, time): return time * speed
def CalcVerPos(initialPos, time): return initialPos - GRAVITY * time**2 / 2
def MetersToPx(pos_m): return pos_m * 15 / 70
def PxToMeters(pos_px): return pos_px * 70 / 15
def DrawPlane(surface, x, y, color):
    x, y = int(x), int(y)
    pygame.draw.rect(surface, color, (x, y, 13, 3))
    pygame.draw.polygon(surface, color, [(x, y + 3), (x, y + 4), (x + 1, y + 3)])
    pygame.draw.polygon(surface, color, [(x + 13, y), (x + 13, y + 1), (x + 15, y)])

def ConvertHeightToScreenY(height): return HEIGHT - MetersToPx(height)

tempoTotalQueda = (h * 2 / GRAVITY) ** 0.5
deltaS = velAviao * tempoTotalQueda

posAlvo_px = random.randint(int(WIDTH * 0.7), int(WIDTH * 0.9))
posAlvo_m = PxToMeters(posAlvo_px)
posLancamento_m = posAlvo_m - deltaS
posLancamento_px = MetersToPx(posLancamento_m)

pygame.init()
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Simulador")
clock = pygame.time.Clock()

simulando = True
cargaLancada = False
tLancamento = 0
alturaAviao_px = ConvertHeightToScreenY(h)
inicioSimulacao = pygame.time.get_ticks()

while simulando:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            simulando = False

    screen.fill((20, 20, 40))

    pygame.draw.rect(screen, (255, 0, 0), (posAlvo_px - 3, HEIGHT - 5, 8, 5))

    tAtual = (pygame.time.get_ticks() - inicioSimulacao) / 1000.0

    posAviao_m = CalcHorPos(velAviao, tAtual)
    posAviao_px = MetersToPx(posAviao_m)
    DrawPlane(screen, posAviao_px, alturaAviao_px, (220, 220, 220))

    if posAviao_px >= posLancamento_px and not cargaLancada:
        cargaLancada = True
        tLancamento = tAtual

    if cargaLancada:
        tempo_de_queda = tAtual - tLancamento

        posCarga_m = posLancamento_m + CalcHorPos(velAviao, tempo_de_queda)
        posCarga_px = MetersToPx(posCarga_m)

        altCarga_m = CalcVerPos(h, tempo_de_queda)
        altCarga_px = ConvertHeightToScreenY(altCarga_m)

        pygame.draw.circle(screen, (255, 255, 255), (posCarga_px, altCarga_px), 2)

        if altCarga_m <= 0:
            simulando = False

    pygame.display.flip()
    clock.tick(60)

pygame.quit()