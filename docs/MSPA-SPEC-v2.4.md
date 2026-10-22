# MSPA-Lib — Especificación lógica v2.4

## Estado

**Arquitectura candidata. No congelada. No existe implementación de producción.**

MSPA es un **Motor de Simulación Psicológica y Cultural de Agentes** pensado como middleware desacoplado del juego. Su trabajo es interpretar estímulos desde la perspectiva subjetiva de un agente, mantener su estado psicológico y devolver preferencias de conducta explicables. El host conserva la autoridad sobre el mundo, la percepción física, las acciones realmente disponibles y su ejecución.

La v2.4 corrige y formaliza los puntos que seguían ambiguos en v2.3: identidad estable de acciones, retorno de urgencia tras observación, evaluación temporal perezosa, llamadas por lotes deterministas, semántica explícita de Domain Packs, lenguaje de inferencia acotado, seguridad frente a estados externos no confiables y una primera versión deliberadamente simple del aprendizaje causal.

---

# 1. Principio fundamental

```text
HOST / MUNDO
    ↓
OBSERVE
    ↓
MSPA actualiza al agente
    ↓
DECIDE
    ↓
MSPA elige entre opciones ofrecidas por el host
    ↓
HOST ejecuta
    ↓
REPORT
    ↓
MSPA aprende y reevalúa
```

Además:

- todas las operaciones relevantes llevan tiempo lógico;
- `ADVANCE_TO` permite actualizar explícitamente agentes dormidos o procesar hitos temporales;
- `EXPORT / IMPORT` mueve o persiste estados psicológicos;
- las operaciones batch son semánticamente equivalentes a procesar agentes en orden canónico.

> **MSPA procesa significado psicológico; el host controla realidad y ejecución.**

---

# 2. Frontera host / MSPA

## El host decide

- qué existe;
- qué ocurrió realmente;
- qué puede percibir cada agente;
- qué acciones son físicamente posibles;
- pathfinding, animación, inventario, combate, navegación, recursos y física;
- qué conceptos y predicados de dominio existen;
- qué reglas de inferencia específicas del dominio se registran;
- qué resultado real produjo una acción.

## MSPA decide

- qué atención presta el agente a un estímulo;
- qué proposiciones y creencias mantiene;
- cómo cambia su certeza;
- qué hipótesis infiere mediante reglas declarativas;
- cómo evalúa subjetivamente un evento;
- qué emociones aparecen;
- cómo cambian metas, ánimo, relaciones y memoria;
- qué espera que ocurra;
- qué acción prefiere entre las ofrecidas;
- qué desea expresar o comunicar;
- por qué tomó esa decisión.

---

# 3. Identidad y espacios de nombres

Los identificadores deben ser opacos al núcleo pero portables entre mundos.

Se distinguen conceptualmente:

- `WorldId` / `NamespaceId`
- `AgentId`
- `EntityId`
- `EventId`
- `ActionTypeId`
- `ActionInstanceId`
- `ActionContinuityKey`
- `GoalId`
- `BeliefId`
- `MemoryId`
- `PredicateId`
- `ConceptId`
- `PackId`

Una referencia portable debe poder distinguir, por ejemplo:

```text
world-a:agent-27
world-b:agent-27
```

Las referencias externas pueden permanecer colgantes sin ser eliminadas. Un agente que migra de mundo puede recordar entidades o lugares que el nuevo host no puede resolver.

---

# 4. Acción: tipo, instancia y continuidad

La histéresis requiere reconocer que una opción nueva representa la continuación de una actividad ya iniciada.

MSPA distingue:

## 4.1 Tipo de acción

Describe la clase semántica de conducta.

Ejemplos:

```text
mspa:communicate
mspa:avoid
isla:harvest
```

## 4.2 Instancia de acción

Describe una ejecución concreta.

Ejemplo:

> hablar con Mara en este encuentro.

## 4.3 Clave de continuidad

La proporciona el host y debe mantenerse estable mientras una misma actividad lógica continúe, aunque el host regenere la lista de opciones en cada consulta.

MSPA usa esta clave para calcular:

- progreso ya realizado;
- coste de interrupción;
- compromiso con la acción actual;
- histéresis;
- persistencia de meta.

Un `ActionId` efímero no puede usarse como única identidad de continuidad.

---

# 5. Personalidad

Se mantienen seis ejes base:

| Eje | Bajo | Alto | Función |
|---|---|---|---|
| Sociabilidad | reservado | sociable | valor subjetivo de interacción |
| Estabilidad | reactivo | sereno | intensidad y recuperación emocional |
| Empatía | egocéntrico | empático | relevancia del estado ajeno |
| Persistencia | flexible | obstinado | continuidad de metas y resistencia al abandono |
| Curiosidad | tradicional | curioso | tolerancia a incertidumbre y exploración |
| Audacia | prudente | temerario | peso subjetivo del riesgo |

Persistencia **no controla directamente** todos los fenómenos de rigidez. Se derivan magnitudes distintas:

- persistencia de meta: principalmente Persistencia + Estabilidad + inversión previa;
- rigidez de creencia: Persistencia − Curiosidad + carga emocional + coherencia identitaria;
- resistencia a cambiar de acción: Persistencia + progreso + coste de interrupción.

Así se evita convertir “obstinado” en una propiedad universal.

---

# 6. Valores

Los valores expresan qué considera importante un agente. El catálogo pertenece al Core Pack o al Domain Pack.

Cada valor tiene al menos:

- identificador;
- intensidad;
- posibles relaciones de conflicto;
- etiquetas semánticas opcionales.

La personalidad describe **cómo** tiende a reaccionar; los valores describen **qué** le importa.

---

# 7. Necesidades y estados internos

Las necesidades pertenecen principalmente al host y se entregan a MSPA como estímulos o estado consultable.

Una necesidad no es una emoción.

Puede convertirse en motivador, meta o factor de appraisal.

---

# 8. Metas

Cada meta contiene conceptualmente:

- origen;
- condición objetivo;
- prioridad;
- estado;
- condición de cierre;
- timestamp de creación;
- posible fecha límite;
- posibilidad de suspensión.

Orígenes posibles:

- necesidad;
- valor;
- compromiso;
- evento;
- creencia;
- relación;
- regulación emocional;
- inspiración o influencia.

Se favorecen pocas metas dominantes simultáneas. El resto puede quedar suspendido o latente.

---

# 9. Proposiciones

Toda creencia se expresa mediante una proposición estructurada.

Una proposición contiene conceptualmente:

- `PredicateId`;
- argumentos con roles;
- polaridad;
- tiempo o intervalo;
- modificadores semánticos;
- namespace;
- procedencia.

Ejemplo:

```text
predicate: mspa:present_at
roles:
  actor: Torvando
  location: Casa_Mara
  event: Incendio_17
polarity: true
```

Otro ejemplo:

```text
predicate: mspa:failed_to_help
roles:
  actor: Torvando
  target: Mara
  event: Incendio_17
intentionality: unknown
```

---

# 10. Vocabulario base MSPA

Para permitir intercambio entre juegos y mundos, MSPA define un vocabulario semántico mínimo compartido.

La lista exacta se validará en sandbox, pero debe cubrir relaciones cognitivamente generales como:

- presencia;
- causalidad;
- ayuda;
- daño;
- amenaza;
- prevención;
- fallo de prevención;
- posesión;
- pertenencia;
- promesa;
- aprobación/desaprobación;
- expectativa;
- deseo;
- permiso/prohibición;
- comunicación;
- observación.

Los juegos extienden este vocabulario mediante namespaces propios.

Ejemplo:

```text
mspa:helped
isla:harvested
rpg:cast_spell
```

---

# 11. Predicate Schema

Un predicado de dominio no es sólo un nombre. El Domain Pack debe declarar su semántica mínima.

Un esquema puede definir:

- roles requeridos y opcionales;
- qué rol tiene agencia;
- qué roles pueden ser objetivo;
- categorías de consecuencia;
- posibles relevancias;
- información sobre intención;
- compatibilidad con predicados base;
- etiquetas de aprendizaje;
- pistas de appraisal.

Ejemplo conceptual:

```text
isla:harvested
roles:
  actor
  resource
  owner?
  location?
semantic_effects:
  resource_gain
  labor_success
  ownership_interaction
agency_role:
  actor
possible_relevance:
  prosperity
  hunger
  ownership
```

El esquema **no** declara que cosechar sea “positivo”. Su valoración depende del agente y del contexto.

---

# 12. Percepción y OBSERVE

El host entrega sólo aquello que el agente pudo percibir o recibir por otro canal.

MSPA aplica filtros de:

- atención;
- relevancia;
- saliencia.

`OBSERVE` actualiza el estado psicológico pero no ejecuta una decisión por sí mismo.

Sin embargo, debe devolver una pista de scheduling:

- `state_changed`;
- `decision_recommended`;
- `urgency`;
- `current_action_threatened`.

Así un ataque, incendio o miedo intenso puede provocar una reevaluación inmediata sin acoplar MSPA al loop del host.

---

# 13. Creencias

Una creencia mantiene:

- proposición;
- certeza;
- fuente;
- origen;
- carga emocional;
- timestamps;
- evidencias a favor/en contra resumidas;
- posibles dependencias de inferencia.

Fuentes típicas:

- observación;
- testimonio;
- inferencia;
- recuerdo;
- inspiración/influencia;
- aprendizaje causal.

El agente reacciona a sus creencias, no a la verdad global.

---

# 14. Inferencia

La inferencia produce **hipótesis**, no verdades.

Las reglas de inferencia deben ser **datos declarativos** y portables. No se permite depender de código arbitrario del host para representar la regla misma.

Ejemplo conceptual:

```text
IF
    A mspa:present_at E
    B mspa:in_danger E
    A mspa:able_to_help B
    A mspa:left E
THEN
    hypothesis:
        A mspa:failed_to_help B
    base_confidence:
        medium
```

---

# 15. Lenguaje de reglas acotado

El lenguaje de inferencia no será Turing-completo.

Requisitos:

- declarativo;
- sin acceso a red o filesystem;
- sin ejecución de código nativo;
- terminación garantizada;
- profundidad máxima de inferencia;
- límite de reglas disparadas por estímulo;
- límite de hipótesis nuevas;
- sin recursión ilimitada;
- ciclos detectables y acotados;
- presupuesto computacional explícito.

El diseño puede inspirarse en un subconjunto tipo Datalog, pero no se fija todavía sintaxis ni implementación.

---

# 16. Actualización y sesgo de creencias

La evidencia no sustituye automáticamente una creencia.

Pesan:

- certeza previa;
- carga emocional;
- confianza y respeto hacia la fuente;
- claridad perceptiva;
- rigidez derivada;
- Curiosidad;
- emociones actuales;
- coherencia con otras creencias;
- independencia de fuentes.

Esto permite sesgo de confirmación sin convertirlo en una regla especial por personaje.

---

# 17. Evaluación cognitiva

MSPA evalúa la **interpretación subjetiva** del agente.

Factores principales:

- relevancia;
- consecuencia;
- agencia;
- intención percibida;
- certeza;
- control;
- norma;
- relación;
- expectativa previa.

La novedad se deriva preferentemente del **error de predicción** entre expectativa y resultado, no de una variable mágica de sorpresa.

---

# 18. Emociones

Se mantienen seis emociones base candidatas:

- Alegría;
- Tristeza;
- Miedo;
- Ira;
- Ansiedad;
- Aversión.

Son instancias con:

- intensidad;
- objetivo;
- causa percibida;
- creencia asociada;
- timestamp;
- función de decaimiento.

Las emociones derivadas (orgullo, culpa, gratitud, duelo, resentimiento, alivio, esperanza, etc.) se calculan a partir del estado y no requieren barras independientes.

La distribución emocional debe validarse para evitar que una población sana derive sistemáticamente hacia un estado negativo.

---

# 19. Ánimo y reapreciación

El ánimo usa dos dimensiones internas candidatas:

- valencia;
- activación.

Las emociones pueden decaer de forma perezosa mediante timestamps.

La reapreciación permite que una memoria conserve el mismo contenido factual pero cambie su carga emocional con el tiempo.

---

# 20. Relaciones

Las relaciones son direccionales y contienen inicialmente:

- afecto;
- confianza;
- respeto;
- familiaridad.

`A → B` y `B → A` son estados diferentes.

La deuda social continúa modelándose como memoria, compromiso o meta, no como quinta barra relacional.

---

# 21. Memoria

Una memoria conserva:

- evento o proposición principal;
- interpretación;
- emoción original;
- importancia;
- fuente;
- certeza;
- participantes;
- timestamp;
- reactivaciones;
- referencias externas.

La consolidación depende de una combinación de:

- intensidad;
- relevancia;
- recurrencia;
- consecuencias prolongadas.

No todas las memorias se conservan.

---

# 22. Aprendizaje causal v1

La primera versión del aprendizaje causal debe ser deliberadamente simple y acotada.

Se modela mediante estadísticas sobre:

```text
ActionType
+
ContextTags
→
OutcomeTags
```

Cada patrón puede mantener:

- intentos;
- éxitos;
- frecuencia de resultados;
- confianza;
- última activación.

El Domain Pack declara qué etiquetas de contexto son relevantes para cada tipo de acción.

No se intenta inicialmente una generalización cognitiva abierta ni aprendizaje simbólico arbitrario.

---

# 23. Exploración frente a explotación

Curiosidad interviene como tolerancia a incertidumbre.

Un agente con Curiosidad alta puede preferir ocasionalmente opciones con menor evidencia previa pero mayor información potencial.

Un agente con Curiosidad baja prioriza prácticas conocidas y expectativas consolidadas.

La exploración debe seguir limitada por riesgo, coste y otras metas.

---

# 24. Aprendizaje social

Un agente puede actualizar expectativas al:

- observar a otro;
- recibir consejo;
- escuchar un testimonio;
- imitar a una persona respetada;
- observar consecuencias sociales.

El peso depende de:

- confianza;
- respeto;
- familiaridad;
- prestigio percibido;
- claridad de la observación;
- compatibilidad con creencias previas.

---

# 25. Cultura emergente

MSPA no mantiene una entidad global llamada “cultura” que controle individuos.

La cultura se entiende como distribución de:

- valores;
- creencias normativas;
- expectativas;
- prácticas aprendidas;
- símbolos y conceptos compartidos.

Cada agente mantiene sus propias creencias sobre lo que el grupo espera.

Herramientas externas pueden medir convergencia, divergencia, subculturas, polarización y transmisión generacional.

La emergencia de cultura rica o religión se considera **hipótesis de diseño a validar**, no promesa del sistema.

---

# 26. Influencia externa e inspiración

MSPA admite fuentes de influencia externas sin hardcodear un “Dios”.

Una `InfluenceSource` puede originar un estímulo.

El agente puede interpretarlo como:

- mandato legítimo;
- intuición;
- amenaza;
- ilusión;
- inspiración;
- mensaje religioso;
- manipulación.

La aceptación depende de personalidad, valores, creencias, relaciones y evidencia previa.

---

# 27. DECIDE

`DECIDE` funciona aunque no haya habido un `OBSERVE` inmediatamente anterior.

El host entrega opciones realmente disponibles.

MSPA no inventa acciones ejecutables que el host no haya ofrecido.

Cada acción puede declarar:

- tipo semántico;
- instancia;
- clave de continuidad;
- actor;
- objetivo;
- efectos esperados;
- costes;
- riesgos;
- tags;
- consideraciones específicas.

---

# 28. Utility AI por consideraciones

No se suman indiscriminadamente todos los sistemas psicológicos para todas las acciones.

Cada tipo de acción declara qué **consideraciones** son relevantes.

Cada consideración transforma una entrada normalizada mediante una curva de respuesta.

Las curvas candidatas deben ser serializables, editables y reproducibles; se prefieren tablas lineales por tramos frente a funciones transcendentes.

Se distinguen conceptualmente:

- contribuciones positivas/negativas;
- condicionantes que reducen fuertemente la viabilidad psicológica;
- restricciones reales del host, que eliminan opciones antes de MSPA.

La personalidad modifica sensibilidad y curvas, no entrega bonificaciones rígidas del tipo “+20 a cazar”.

---

# 29. Histéresis

Una acción en curso recibe ventaja de continuidad según:

- progreso;
- coste de interrupción;
- persistencia de meta;
- compromiso;
- estabilidad contextual.

Se abandona cuando:

- termina;
- deja de ser posible;
- una emergencia exige reevaluación;
- otra alternativa supera suficientemente el umbral de cambio.

La clave de continuidad es obligatoria para que esta lógica sea estable entre llamadas.

---

# 30. REPORT

El host informa el resultado real de una acción.

Puede incluir:

- éxito;
- fracaso;
- resultado parcial;
- efectos observados;
- consecuencias inesperadas;
- causa de interrupción;
- recursos obtenidos/perdidos;
- terceros afectados.

MSPA usa `REPORT` para:

- aprendizaje causal;
- actualización de metas;
- emociones;
- memoria;
- relaciones;
- actualización de expectativas.

---

# 31. Tiempo y evaluación perezosa

Todas las operaciones relevantes incluyen tiempo lógico.

Emociones, memoria, ánimo y otros estados no necesitan actualizarse cada frame.

El estado efectivo puede derivarse de:

- valor original;
- timestamp;
- tasa o curva de decaimiento;
- reactivaciones.

Esto reduce el coste y evita un “tick psicológico completo”.

---

# 32. ADVANCE_TO

`ADVANCE_TO` no es un tick obligatorio.

Se conserva para:

- reactivar agentes dormidos;
- procesar metas vencidas;
- resolver hitos temporales;
- actualizar largos periodos fuera de simulación detallada;
- consultar si el paso del tiempo por sí solo hace recomendable una nueva decisión.

En sistemas simples puede no llamarse regularmente si otras operaciones ya llevan timestamps suficientes.

---

# 33. Niveles de simulación

El host puede asignar un nivel de actividad:

### Activo

Procesamiento completo cuando es necesario.

### Reducido

Sólo estímulos relevantes y decisiones menos frecuentes.

### Dormido

Estado conservado mediante marcas temporales y actualización diferida.

Cambiar de nivel no debe cambiar retroactivamente la identidad psicológica del agente.

---

# 34. Procesamiento por lotes

MSPA debe admitir conceptualmente operaciones multiagente para rendimiento.

Una llamada batch debe producir resultados equivalentes a procesar individualmente los agentes en un **orden canónico estable**.

El paralelismo interno no puede cambiar el resultado observable.

Los agentes no comparten RNG mutable global.

---

# 35. Determinismo

Objetivo:

> mismo estado + mismos packs + mismas entradas + mismos timestamps + mismas seeds = mismo resultado lógico.

Requisitos:

- orden de iteración definido cuando afecte resultados;
- RNG independiente por agente o stream derivado;
- seeds estables;
- ausencia de dependencia accidental del orden de threads;
- curvas reproducibles;
- versionado de packs y estado.

No se promete determinismo bit-a-bit multiplataforma hasta elegir política numérica concreta.

---

# 36. Estado externo no confiable

Todo `AgentPsychState` importado debe considerarse entrada potencialmente hostil.

MSPA debe validar no sólo estructura, sino presupuestos.

Deben existir límites configurables para:

- número de creencias;
- número de memorias;
- número de metas;
- número de relaciones;
- tamaño de strings;
- profundidad de proposiciones;
- cantidad de reglas referenciadas;
- tamaño total de estado;
- hipótesis generadas por procesamiento;
- profundidad de derivación;
- complejidad de contexto de aprendizaje.

Un estado que exceda límites debe rechazarse, truncarse de forma explícita o importarse bajo una política definida; nunca consumirse sin control.

---

# 37. Domain Packs

El producto de integración de cada juego es un **Domain Pack**.

Debe declarar como datos, no como comportamiento oculto:

- namespace;
- versión;
- dependencias;
- vocabulario de dominio;
- Predicate Schemas;
- Event Templates;
- tipos de acción;
- roles semánticos;
- consideraciones de utilidad;
- curvas de respuesta;
- reglas de inferencia;
- ContextTags de aprendizaje;
- OutcomeTags;
- conceptos exportables;
- compatibilidad con otros packs;
- presupuestos y límites permitidos.

MSPA debe incluir además un **Core Psychology Pack** con vocabulario y reglas generales mínimas.

---

# 38. Compatibilidad entre packs

Cada pack posee:

- `pack_id`;
- versión;
- namespace;
- dependencias;
- versión mínima/máxima compatible cuando corresponda;
- hashes o identidad de contenido cuando la implementación lo permita.

Al importar un agente:

- conceptos conocidos se resuelven;
- conceptos desconocidos pueden permanecer opacos;
- referencias externas se preservan;
- reglas no autorizadas no se ejecutan;
- incompatibilidades se reportan explícitamente.

---

# 39. Migración entre mundos

Un agente migrante debe poder conservar:

- personalidad;
- valores;
- metas portables;
- memorias;
- creencias;
- relaciones externas;
- expectativas aprendidas;
- prácticas y normas percibidas.

El nuevo host no necesita comprender todos sus conceptos para conservarlos.

---

# 40. Salida de decisión

Una decisión puede devolver:

- acción seleccionada;
- objetivo;
- confianza de decisión;
- intención;
- emoción expresada;
- afirmaciones que desea comunicar;
- explicación causal;
- continuidad esperada.

El host traduce esta intención a animación, diálogo, navegación y ejecución.

---

# 41. Explicabilidad

MSPA debe poder responder:

> ¿por qué este agente hizo esto?

La explicación debe citar factores realmente usados por el proceso decisional, por ejemplo:

```text
Elegir: evitar a Torvando

aversión hacia Torvando      alta
confianza en Torvando        baja
meta: evitar conflicto       relevante
norma comunitaria            frena confrontación
acción actual                sin progreso significativo
```

La explicación es requisito funcional, no accesorio de debug.

---

# 42. Trazas

Un modo diagnóstico debe permitir observar, bajo límites configurables:

```text
OBSERVED
→ proposition
→ belief update
→ inference
→ appraisal
→ emotion
→ memory activation
→ candidate actions
→ considerations
→ chosen action
→ report
→ learning update
```

---

# 43. Degradación modular

Si un módulo opcional está desactivado, el comportamiento debe permanecer definido.

Ejemplos:

- sin memoria: no hay reactivación episódica, pero relaciones y emociones siguen funcionando;
- sin relaciones: se usa estado social neutral por defecto;
- sin inferencia avanzada: sólo creencias directas/testimoniales;
- sin ánimo: emociones siguen actuando;
- sin regulación: no se generan estrategias de coping;
- sin aprendizaje: las expectativas proceden sólo de packs o creencias iniciales;
- sin expresión: el estado interno existe pero no se generan señales emocionales externas.

---

# 44. Sandbox antes de congelar

La arquitectura no se congela hasta superar un sandbox ejecutable.

Los escenarios deben expresarse como datos neutrales y reutilizarse luego como suite de conformidad.

Casos mínimos:

1. Mara: cuatro perfiles reciben evidencia contradictoria y divergen sin reglas especiales.
2. Inspiración: agentes diferentes interpretan una misma influencia de forma distinta.
3. Viajero: un agente migra entre mundos conservando referencias y expectativas desconocidas para el destino.
4. Histéresis: opciones casi empatadas no generan oscilación.
5. Determinismo: mismos datos producen misma secuencia lógica.
6. Batch: individual y batch producen resultados equivalentes.
7. Estado hostil: un estado sobredimensionado es rechazado o limitado de forma definida.
8. Aprendizaje: un agente forma una expectativa simple `acción + tags → resultado` y la actualiza tras éxitos y fallos.
9. Cultura: dos poblaciones con seeds distintas pueden divergir y, al entrar en contacto, modificar creencias y prácticas sin una regla especial `culture_conflict`.

---

# 45. Isla Viva como prueba de contrato

Antes de escribir el runtime de producción debe existir un `IslaVivaPsychPack` suficientemente completo en datos/diseño.

Debe demostrar que puede representar al menos:

- hambre, sueño, familia, trabajo y vivienda;
- relaciones familiares y sociales;
- nacimiento, muerte y duelo;
- llegada de barcos y extranjeros;
- aprendizaje de trabajo y hábitos;
- transmisión de creencias;
- influencia divina;
- viaje de habitantes entre islas;
- choque de normas entre comunidades.

Si estos casos exigen excepciones dentro del núcleo MSPA, el contrato todavía no está listo.

---

# 46. Candidato técnico, no decisión congelada

Para el sandbox se considera candidato **Python**, por velocidad de iteración, análisis y visualización.

Para producción se considera actualmente candidato **Rust con ABI C estable** como frontera de integración, debido a:

- seguridad de memoria al procesar estados/packs externos;
- tipos adecuados para estados versionados;
- facilidad para encapsular concurrencia segura;
- capacidad de integrar desde C/C++/C#/Godot/Unity/Python;
- posibilidad futura de WebAssembly.

Esto **no autoriza todavía la implementación** ni congela lenguaje, ABI, formato de serialización o build system. Debe validarse contra el Domain Pack y el sandbox.

---

# 47. Estado de congelación

La v2.4 es una **arquitectura candidata implementable**, no una API final.

Antes de congelar deben quedar demostrados:

- esquema de proposiciones;
- vocabulario base;
- inferencia declarativa acotada;
- utilidad por consideraciones;
- continuidad de acciones;
- retorno de urgencia de `OBSERVE`;
- aprendizaje causal acotado;
- determinismo individual y batch;
- importación segura;
- Domain Pack realista de Isla Viva;
- escenarios contractuales en sandbox.

Sólo después debe fijarse una API binaria o pública definitiva.
