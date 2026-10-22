# MSPA-Lib

**MSPA — Motor de Simulación Psicológica de Agentes**  
**Agent Psychology Simulation Middleware**

MSPA-Lib es una librería conceptual para dotar agentes autónomos de personalidad, valores, metas, creencias subjetivas, emociones, ánimo, memoria, relaciones y preferencias de acción explicables.

> Estado actual: **diseño lógico**. No existe implementación de código todavía.

La librería está pensada para ser desacoplada del motor y del género de juego. No debe conocer sprites, pathfinding, inventarios concretos, escenas, animaciones, navegación, combate ni conceptos propios de un proyecto específico. El proyecto anfitrión describe lo que ocurre y qué acciones son posibles; MSPA interpreta esos datos desde la perspectiva de cada agente y devuelve preferencias de conducta y estado psicológico.

## Contrato conceptual

```text
MUNDO / JUEGO
    ↓
acontecimientos percibidos + acciones disponibles
    ↓
MSPA-Lib
    ↓
creencias + emociones + metas + preferencias + explicación
    ↓
MUNDO / JUEGO
```

El juego dice **qué ocurrió, qué pudo percibirse y qué puede hacerse**. MSPA decide **qué cree el agente que ocurrió, cómo lo interpreta, qué siente y qué prefiere hacer**. El juego ejecuta la acción y devuelve el resultado.

## Principios de diseño

- El agente reacciona a **lo que cree**, no a la verdad global del juego.
- Personalidad, valores, metas, emociones, ánimo, memoria y relaciones cumplen funciones distintas.
- Las emociones modifican decisiones; no sustituyen la toma de decisiones.
- La personalidad modifica sensibilidades y pesos; no actúa como una tabla de bonificaciones directas.
- Las creencias tienen fuente, certeza y pueden ser erróneas.
- Las memorias almacenan interpretación y carga emocional, no sólo hechos.
- Las relaciones son direccionales.
- El sistema debe ser determinista con el mismo estado inicial, eventos, tiempo y semilla.
- La API futura deberá ser pequeña y agnóstica al motor.
- La explicación causal de una decisión forma parte del diseño, no es una herramienta secundaria.

## Núcleo psicológico previsto

1. Personalidad: 6 ejes.
2. Valores configurables.
3. Necesidades aportadas por el host.
4. Metas activas y compromisos.
5. Percepción y atención.
6. Creencias subjetivas.
7. Evaluación cognitiva.
8. 6 emociones básicas y emociones derivadas.
9. Regulación emocional.
10. Emoción sentida y emoción expresada.
11. Ánimo mediante valencia y activación.
12. Relaciones: afecto, confianza, respeto y familiaridad.
13. Memoria selectiva, reactivación y reapreciación.
14. Normas culturales.
15. Decisión por utilidad, histéresis y ruido bajo.
16. Sesgo de confirmación.

## Alcance deliberadamente excluido por ahora

No forman parte del núcleo inicial: ideologías complejas, enfermedades mentales, personalidad política, árbol de rasgos, deuda social como estadística, ánimo colectivo, contagio emocional, reputación pública compleja, identidad narrativa ni un sistema de rumores independiente.

La especificación detallada está en [`docs/MSPA-SPEC-v2.1.md`](docs/MSPA-SPEC-v2.1.md).
