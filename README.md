# MSPA-Lib

**MSPA — Motor de Simulación Psicológica y Cultural de Agentes**  
**Agent Psychology Simulation Middleware**

MSPA-Lib es una librería conceptual para dotar agentes autónomos de personalidad, valores, metas, creencias subjetivas, emociones, ánimo, memoria, relaciones, aprendizaje y preferencias de acción explicables. También busca permitir que prácticas, normas y cultura puedan emerger de la interacción entre agentes sin convertir “la cultura” en una entidad omnisciente separada.

> Estado actual: **diseño lógico / arquitectura candidata v2.4 + planificación de arquitectura de código v0.1**. No existe implementación de producción todavía.

La librería está pensada para ser desacoplada del motor y del género de juego. El host describe qué ocurre, qué puede percibirse, qué acciones son realmente posibles y cuál fue su resultado. MSPA interpreta esos datos desde la perspectiva subjetiva de cada agente y devuelve estado psicológico, intención y explicación.

Los juegos consumidores están **fuera** de MSPA. MSPA proporciona el core, el contrato y conectores genéricos; cada proyecto externo realiza su propio mapeo de entidades, eventos y acciones.

## Contrato conceptual

```text
HOST / MUNDO
    ↓ OBSERVE
estímulos percibidos
    ↓
MSPA actualiza estado interno
    ↓ DECIDE
opciones reales del host
    ↓
MSPA devuelve intención + expresión + explicación
    ↓
HOST ejecuta
    ↓ REPORT
resultado real
    ↓
MSPA aprende / actualiza
```

Además:

- todas las operaciones relevantes llevan tiempo lógico entero;
- `ADVANCE_TO` existe para agentes dormidos, hitos y actualización temporal explícita, no como tick obligatorio;
- `EXPORT / IMPORT` permite persistir o trasladar estado psicológico entre runtimes o mundos;
- existen formas batch conceptuales con orden canónico y determinismo equivalente al procesamiento individual.

## Principios de diseño

- El agente reacciona a **lo que cree**, no a la verdad global del juego.
- Percepción, creencia, inferencia y reapreciación son procesos separados.
- Las creencias se expresan mediante proposiciones semánticas.
- MSPA define un vocabulario base y los juegos lo extienden mediante namespaces.
- Las inferencias usan reglas declarativas, acotadas, portables y no Turing-completas.
- La personalidad modifica sensibilidades y curvas; no entrega bonificaciones rígidas a acciones.
- MSPA selecciona sólo entre acciones ofrecidas por el host.
- La histéresis usa una identidad estable de continuidad, no IDs efímeros.
- `OBSERVE` puede recomendar una reevaluación urgente sin tomar la decisión por el host.
- El aprendizaje causal inicial se limita a `ActionType + ContextTags → OutcomeTags`.
- Las relaciones son direccionales.
- La cultura se interpreta como distribución de creencias, valores, expectativas y prácticas entre agentes.
- El runtime debe ser explicable, reproducible, portable y resistente a datos externos no confiables.
- Los estados y packs importados deben respetar límites de tamaño y complejidad.
- El diseño debe escalar mediante evaluación perezosa, niveles de simulación y batching determinista.
- MSPA define el **contrato de Domain Pack**; los Domain Packs concretos pertenecen a los proyectos consumidores.

## Arquitectura vigente

La versión lógica actual es **MSPA v2.4 — arquitectura candidata**. La planificación de código actual es **v0.1**. Ninguna está congelada todavía.

Antes de fijar una API de producción deben validarse en sandbox:

- lenguaje de proposiciones;
- vocabulario base;
- inferencia declarativa acotada;
- composición de utility AI por consideraciones y curvas;
- continuidad e histéresis;
- retorno de urgencia de `OBSERVE`;
- aprendizaje causal simple;
- determinismo individual y batch;
- migración de agentes entre mundos;
- importación segura de estados y packs.

## Domain Packs

Un juego no modifica el core para enseñarle sus conceptos. El proyecto consumidor mantiene externamente un Domain Pack con:

- vocabulario propio;
- Predicate Schemas;
- Event Templates;
- Action Schemas;
- reglas de inferencia;
- consideraciones y curvas;
- ContextTags y OutcomeTags de aprendizaje;
- compatibilidad y namespaces.

MSPA define el formato, lo valida y lo consume. El pack concreto no vive dentro de MSPA.

## Candidatos técnicos

Todavía **no se ha autorizado implementación de producción**.

Candidatos actuales:

- **Sandbox:** escenarios neutrales reutilizables después como suite de conformidad; la herramienta concreta puede ser desechable.
- **Producción:** C++ nativo como candidato principal, con fachada C opcional para bindings externos.

Todavía no están congelados ABI, serialización, build system, threading model ni bindings.

## Documentación vigente

- [`docs/MSPA-SPEC-v2.4.md`](docs/MSPA-SPEC-v2.4.md) — arquitectura lógica candidata actual.
- [`docs/MSPA-INTERACTION-CONTRACT-v0.2.md`](docs/MSPA-INTERACTION-CONTRACT-v0.2.md) — contrato `OBSERVE / DECIDE / REPORT / ADVANCE_TO / EXPORT-IMPORT`.
- [`docs/MSPA-DOMAIN-PACK-CONTRACT-v0.1.md`](docs/MSPA-DOMAIN-PACK-CONTRACT-v0.1.md) — contrato de integración semántica por juego.
- [`docs/MSPA-CODE-ARCHITECTURE-v0.1.md`](docs/MSPA-CODE-ARCHITECTURE-v0.1.md) — planificación de módulos, ownership, dependencias, runtime, conectores y orden de implementación.

### Historial conservado

- [`docs/MSPA-SPEC-v2.3.md`](docs/MSPA-SPEC-v2.3.md)
- [`docs/MSPA-INTERACTION-CONTRACT-v0.1.md`](docs/MSPA-INTERACTION-CONTRACT-v0.1.md)
- [`docs/MSPA-SPEC-v2.1.md`](docs/MSPA-SPEC-v2.1.md)

## Próxima prueba de diseño

Antes de escribir el runtime de producción deben existir escenarios neutrales de conformidad suficientes para validar al menos:

- el caso Mara con perfiles distintos;
- una influencia externa;
- un viajero con referencias/namespaces externos;
- evidencia contradictoria;
- aprendizaje de una acción;
- histéresis entre acciones casi empatadas.

Después, cada proyecto consumidor integrará MSPA desde su propio repositorio mediante los conectores y contratos públicos, sin introducir clases específicas de ese proyecto dentro del core.
