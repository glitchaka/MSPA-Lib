# MSPA-Lib

**MSPA — Motor de Simulación Psicológica y Cultural de Agentes**  
**Agent Psychology Simulation Middleware**

MSPA-Lib es una biblioteca C++17 para mantener estado psicológico subjetivo de agentes, interpretar estímulos, formar y revisar creencias, producir appraisal y afecto, conservar memoria y relaciones, aprender expectativas y elegir entre acciones que el host declara físicamente disponibles.

> MSPA procesa significado psicológico y preferencia conductual; el host controla la realidad, la percepción física y la ejecución.

## Estado

Versión actual: **1.1.3**.

El núcleo implementa:

- múltiples runtimes aislados por proceso;
- agentes con personalidad, valores, creencias, emociones, mood, metas, memorias y relaciones;
- perfiles psicológicos extensibles definidos por el consumidor;
- `OBSERVE`, `DECIDE`, `REPORT`, `ADVANCE_TO`, `REACT` y operaciones batch;
- aprendizaje contextual de resultados propios;
- creación explícita de descendientes con herencia atenuada de aprendizaje;
- Utility AI psicológica e histéresis con límite de inercia;
- serialización, import/export, migración y rebase temporal;
- RNG determinista por agente;
- Domain Packs y semántica estructurada;
- diagnostics explicables;
- API C++ y ABI C para el flujo principal;
- tests en GCC, Clang+ASan/UBSan y MSVC, fuzzing, mutation testing y benchmarks.

### Cambios 1.1.x

**1.1.0 — perfiles psicológicos extensibles.** `PsychologicalProfile`, `ProfileRegistry` y `ProfileRecipe` permiten registrar y combinar perfiles arbitrarios sin presets de juegos dentro del core.

**1.1.1 — baseline emocional tónico.** Todo agente activo mantiene las seis emociones base por encima de cero. El decaimiento vuelve hacia ese baseline en vez de eliminar la emoción.

**1.1.2 — endurecimiento conductual y auditoría.** Se separa baseline de emoción episódica, los fallos reducen expectativa de éxito, se endurecen determinismo, tiempo, serialización, cuotas, C ABI y mutation testing.

**1.1.3 — descendencia y continuidad.** `create_agent_descendant` permite heredar personalidad, valores y expectativas aprendidas atenuadas. Los recuerdos autobiográficos, las relaciones personales y las creencias personales no se copian al descendiente. La histéresis limita su inercia total a `0.35`; una urgencia máxima la anula completamente. La auditoría incluye mutaciones específicas para eliminar la herencia y restaurar el antiguo doble conteo de histéresis.

## 1. Contrato de integración

MSPA no observa el mundo directamente y no ejecuta acciones físicas. Un consumidor integra el ciclo:

```text
HOST / WORLD
    │ percepción física
    ▼
OBSERVE / REACT
    │
    ▼
MSPA actualiza creencias, appraisal, emociones, mood y memoria
    │
    ▼
DECIDE(opciones físicamente posibles)
    │
    ▼
HOST ejecuta la acción seleccionada
    │
    ▼
REPORT(resultado real)
    │
    ▼
MSPA actualiza expectativas aprendidas
```

Para que un NPC muestre conducta psicológicamente diferenciada, el host debe:

1. enviar los acontecimientos que el NPC percibe mediante `OBSERVE` o `REACT`;
2. ofrecer a `DECIDE` alternativas reales con coste, riesgo, progreso, contexto y efectos esperados significativos;
3. ejecutar la selección de MSPA en el mundo;
4. informar el resultado mediante `REPORT`;
5. volver a ofrecer decisiones cuando cambien el entorno, la urgencia o las alternativas.

Si el host ignora la selección, no reporta resultados o ofrece opciones indistinguibles, MSPA no puede producir por sí solo una conducta visible diferente.

## 2. Responsabilidades

### El host posee

- mundo y entidades físicas;
- percepción física, proximidad y línea de visión;
- pathfinding, navegación, física y colisiones;
- inventario, economía, recursos y combate;
- animación y presentación;
- catálogo de acciones físicamente posibles;
- scheduling;
- consecuencias reales de las acciones;
- semántica concreta del juego o simulación.

### MSPA posee

- personalidad y rasgos derivados;
- valores;
- creencias subjetivas;
- inferencia;
- appraisal;
- emociones y mood;
- metas;
- memoria;
- relaciones direccionales;
- expectativas aprendidas;
- preferencia entre acciones ofrecidas;
- continuidad de intención e histéresis;
- trazas explicables;
- herencia de predisposiciones aprendidas para descendientes.

## 3. Personalidad y perfiles

La personalidad usa seis ejes `[0,1]`:

- sociabilidad;
- estabilidad;
- empatía;
- persistencia;
- curiosidad;
- audacia.

De ellos se derivan rigidez de creencias, persistencia de metas, resistencia al cambio, sensibilidad a amenaza, sensibilidad social y exploración.

MSPA no contiene perfiles concretos hardcodeados. El consumidor puede registrar cuantos `PsychologicalProfile` necesite mediante `ProfileRegistry`, combinarlos con `ProfileRecipe` y añadir variación determinista por agente.

## 4. Percepción y respuesta al entorno

`OBSERVE` recibe una `Proposition` estructurada y un `StimulusTime`. `process_time` es el momento en que MSPA procesa el evento; `event_time` puede representar un hecho anterior.

La observación puede actualizar:

- creencias;
- inferencias;
- appraisal;
- emociones;
- mood;
- memoria;
- recomendación de reconsiderar la decisión actual.

`REACT` combina `OBSERVE + DECIDE` cuando el host quiere una respuesta inmediata a un estímulo.

## 5. Decisión

`DECIDE` sólo selecciona entre `ActionOption` que el host entrega. Una opción puede incluir:

- coste;
- riesgo;
- progreso;
- novedad;
- exposición social;
- coste de interrupción;
- estabilidad contextual;
- `ContextTags`;
- efectos esperados sobre necesidades, valores, metas, relaciones o emoción;
- señales adicionales del consumidor.

`DecisionContext` permite expresar presiones y urgencia sin hardcodear necesidades de un juego dentro del core.

La personalidad, estado emocional episódico, metas, valores, relaciones y aprendizaje pueden cambiar la utilidad relativa de esas opciones.

## 6. Histéresis

`ActionContinuityKey` permite mantener una intención cuando dos acciones están casi empatadas. La continuidad depende de persistencia, estabilidad, progreso, coste de interrupción y estabilidad contextual.

Desde 1.1.3:

- `continuity_bonus + switch_guard <= 0.35` antes del escalado por urgencia;
- urgencia `1.0` elimina la inercia;
- las dos contribuciones permanecen separadas en la traza;
- existe una prueba de perfiles extremos;
- mutation testing restaura deliberadamente el antiguo doble conteo y exige que la suite lo detecte.

MSPA **no implementa actualmente desgaste temporal/frustración de la histéresis**. Una alternativa persistentemente mejor pero con ventaja inferior al límite de inercia puede continuar sin provocar cambio si nada más modifica el estado. Esto es una limitación conocida, no una capacidad certificada.

## 7. Aprendizaje

`REPORT` registra el resultado real de una acción. El patrón básico es:

```text
ActionType + ContextTags -> expectativa de resultado
```

Éxitos repetidos aumentan la expectativa de éxito y fallos repetidos la reducen. Los contextos se canonicalizan para que un agente pueda aprender que una misma acción funciona en un contexto y falla en otro.

El aprendizaje influye posteriormente en `DECIDE`.

Actualmente el core aprende de **resultados propios reportados**. El aprendizaje observacional de lo que le ocurre a otro agente sigue sin estar implementado.

## 8. Descendencia e herencia de aprendizaje

`create_agent_derived` continúa siendo una operación genérica de mezcla y no presupone parentesco.

Para parentesco existe desde 1.1.3:

```cpp
DescendantInit descendant;
descendant.parent_agents = {parent_a, parent_b};
descendant.inherited_learning_policy = InheritedLearningPolicy::Mean;
descendant.inherited_learning_evidence = 0.5f;

runtime->create_agent_descendant(child, descendant, now);
```

`create_agent_descendant` puede heredar:

- personalidad combinada;
- valores combinados;
- expectativas de acción/contexto aprendidas por los progenitores.

El aprendizaje heredado se atenúa hacia neutralidad. Es una **predisposición inicial**, no conocimiento infalible. La experiencia propia del descendiente puede reforzarlo o contradecirlo hasta invertir su preferencia conductual.

No se heredan automáticamente:

- recuerdos autobiográficos;
- relaciones personales;
- creencias personales concretas.

La cantidad de patrones heredados respeta `RuntimeLimits.max_learning_patterns_per_agent` y puede limitarse adicionalmente en `DescendantInit`.

## 9. Emociones

Emociones base:

- Joy;
- Sadness;
- Fear;
- Anger;
- Anxiety;
- Aversion.

Todo agente activo conserva las seis. MSPA distingue baseline tónico y exceso episódico. El baseline no se interpreta como miedo o ira activa para urgencia, riesgo o expresión. La estabilidad modifica también la velocidad de recuperación emocional.

## 10. Persistencia y determinismo

MSPA incluye:

- `PsychTime` entero de 64 bits;
- `TimeDomain` con escala racional;
- RNG independiente por agente;
- `export_agent` / `export_agent_bytes`;
- import con `RebaseRelative` y `RekeyForDestination`;
- serialización binaria versionada;
- migración de estado;
- batches con orden canónico.

No se promete identidad bit-a-bit entre toolchains distintos; sí determinismo lógico bajo el mismo estado, configuración, inputs y seed dentro del contrato probado.

## 11. Build

Requisitos:

- CMake 3.20+;
- C++17;
- MSVC, GCC o Clang.

```bash
cmake -S . -B build -DMSPA_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Consumo instalado:

```cmake
find_package(MSPA 1 CONFIG REQUIRED)
target_link_libraries(mi_programa PRIVATE MSPA::mspa)
```

El alias `MSPA::mspa` también existe en el árbol de construcción, por lo que puede usarse con `FetchContent`.

Opciones:

| Opción | Default | Función |
|---|---:|---|
| `MSPA_BUILD_TESTS` | ON | suites CTest |
| `MSPA_BUILD_BENCHMARKS` | OFF | benchmark OBSERVE/DECIDE |
| `MSPA_BUILD_SHARED` | OFF | biblioteca compartida |
| `MSPA_BUILD_FUZZ` | OFF | harness libFuzzer; requiere Clang |

## 12. Validación 1.1.3

La suite actual incluye pruebas específicas de:

- determinismo;
- perfiles y divergencia conductual;
- aprendizaje contextual;
- baseline y decay emocional;
- histéresis normal y extrema;
- serialización, migración e import;
- cuotas y entrada hostil;
- ABI C del flujo principal;
- herencia de aprendizaje del descendiente.

La prueba de descendencia verifica un ciclo completo:

```text
progenitores aprenden X
        ↓
descendiente hereda una expectativa X atenuada
        ↓
DECIDE del descendiente favorece X antes de experiencia propia
        ↓
X falla repetidamente para el descendiente
        ↓
la expectativa heredada cae por debajo de neutral
        ↓
DECIDE cambia de preferencia
```

Mutation testing incluye ocho regresiones deliberadas, entre ellas:

- eliminar toda histéresis;
- restaurar el doble conteo de histéresis;
- eliminar la herencia de aprendizaje;
- eliminar el baseline emocional;
- romper rebase temporal;
- romper cuotas;
- invertir orden canónico;
- permitir que reapreciación reescriba un hecho.

## 13. Capacidades todavía no implementadas

No deben considerarse cubiertas por proximidad con otras funciones:

- aprendizaje observacional del resultado de otros agentes;
- normas percibidas de grupo;
- generación/transmisión cultural de rumores o afirmaciones;
- relaciones con entidades no-agente;
- métricas poblacionales de deriva cultural, diversidad o polarización;
- desgaste temporal/frustración de histéresis;
- paridad completa de `create_agent_descendant` en la ABI C (la API C++ sí lo expone).

Los documentos normativos y la evidencia de auditoría están en `docs/` y `docs/audit/`.