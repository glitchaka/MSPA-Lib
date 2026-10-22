# MSPA-Lib — Especificación lógica v2.4

## Estado

**Arquitectura candidata, completamente agnóstica al host.**

MSPA es un motor de simulación psicológica y cultural de agentes. Interpreta estímulos desde la perspectiva subjetiva de un agente, mantiene estado psicológico y devuelve preferencias de conducta explicables.

El host conserva autoridad sobre el mundo, percepción física, entidades reales, acciones disponibles y ejecución.

Este documento no contiene personajes, juegos, repositorios, lugares, eventos ni reglas de proyectos consumidores.

---

## 1. Contrato fundamental

```text
HOST
  ↓ OBSERVE
MSPA actualiza estado subjetivo
  ↓ DECIDE
MSPA selecciona entre opciones ofrecidas
  ↓
HOST ejecuta
  ↓ REPORT
MSPA aprende y reevalúa
```

Además:
- todas las operaciones relevantes usan tiempo lógico explícito;
- `ADVANCE_TO` materializa efectos temporales diferidos;
- `EXPORT / IMPORT` permite persistencia y traslado de estado;
- las operaciones batch son equivalentes al procesamiento individual en orden canónico.

> **MSPA procesa significado psicológico; el host controla realidad y ejecución.**

---

## 2. Frontera host / MSPA

### El host decide
- qué existe;
- qué ocurrió realmente;
- qué pudo percibirse;
- qué acciones son físicamente posibles;
- pathfinding, animación, inventario, combate, navegación, recursos y física;
- qué semántica de dominio registra;
- qué resultado real produjo una acción.

### MSPA decide
- qué atención presta el agente;
- qué proposiciones y creencias mantiene;
- cómo cambia su certeza;
- qué hipótesis infiere mediante reglas declarativas;
- cómo evalúa subjetivamente lo percibido;
- qué emociones aparecen;
- cómo cambian metas, ánimo, relaciones y memoria;
- qué espera que ocurra;
- qué acción prefiere entre las ofrecidas;
- qué desea expresar o comunicar;
- por qué tomó esa decisión.

---

## 3. Identidad y namespaces

Identificadores conceptuales:
- `WorldId`
- `NamespaceId`
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

Los IDs son opacos para el core. Las referencias externas pueden preservarse aunque no sean resolubles localmente.

---

## 4. Acción y continuidad

MSPA distingue:
- **tipo de acción:** clase semántica;
- **instancia de acción:** ejecución concreta;
- **clave de continuidad:** identidad estable mientras continúa una misma actividad lógica.

La histéresis depende de la clave de continuidad, progreso, coste de interrupción y estado psicológico, no de IDs efímeros.

---

## 5. Personalidad

Seis ejes base:

| Eje | Bajo | Alto | Función |
|---|---|---|---|
| Sociabilidad | reservado | sociable | peso de interacción |
| Estabilidad | reactivo | sereno | intensidad y recuperación emocional |
| Empatía | egocéntrico | empático | relevancia del estado ajeno |
| Persistencia | flexible | obstinado | continuidad de metas |
| Curiosidad | tradicional | curioso | tolerancia a incertidumbre/exploración |
| Audacia | prudente | temerario | peso subjetivo del riesgo |

La personalidad modifica sensibilidades y pesos. No aplica bonificaciones directas a acciones.

Magnitudes derivadas como rigidez de creencia o resistencia a abandonar una acción combinan varios factores y no dependen de un único eje.

---

## 6. Valores

Los valores expresan qué considera importante un agente.

Cada valor tiene al menos:
- `ConceptId`;
- intensidad;
- posibles conflictos;
- etiquetas semánticas opcionales.

Los catálogos concretos pertenecen a datos externos.

---

## 7. Necesidades y estados internos

Las necesidades pertenecen principalmente al host y llegan como estímulos o estado consultable.

Una necesidad no es una emoción. Puede convertirse en motivador, meta o factor de appraisal.

---

## 8. Metas

Cada meta contiene:
- origen;
- condición objetivo;
- prioridad;
- estado;
- condición de cierre;
- creación;
- posible deadline;
- posibilidad de suspensión.

Orígenes posibles: necesidad, valor, compromiso, evento, creencia, relación, regulación emocional o influencia externa.

Sólo unas pocas metas deben dominar simultáneamente.

---

## 9. Proposiciones

Toda creencia se expresa mediante una proposición estructurada:
- `PredicateId`;
- argumentos por rol;
- polaridad;
- tiempo/intervalo;
- calificadores;
- namespace;
- procedencia.

El formato es genérico y no contiene semántica de un proyecto concreto.

---

## 10. Vocabulario base

MSPA define un vocabulario mínimo de relaciones psicológica y socialmente generales: presencia, causalidad, ayuda, daño, amenaza, prevención, posesión, pertenencia, promesa, aprobación, expectativa, deseo, permiso, prohibición, comunicación y observación.

Los hosts extienden el vocabulario mediante namespaces externos.

---

## 11. Predicate Schema

Un predicado interpretable declara:
- roles requeridos y opcionales;
- agencia;
- posibles objetivos;
- categorías de consecuencia;
- relevancia posible;
- intencionalidad;
- compatibilidad con vocabulario base;
- tags de aprendizaje;
- pistas de appraisal.

Un schema no fija una emoción universal.

---

## 12. Percepción y atención

El host entrega sólo información accesible para el agente.

MSPA aplica filtros de:
- atención;
- relevancia;
- saliencia.

Un estímulo irrelevante puede no producir creencia, emoción ni memoria.

---

## 13. Creencias

Cada creencia mantiene:
- proposición;
- certeza;
- fuente;
- origen;
- carga emocional;
- timestamps;
- evidencia resumida;
- dependencias de inferencia opcionales.

Fuentes genéricas: observación, testimonio, inferencia, recuerdo, influencia externa y aprendizaje.

El agente reacciona a sus creencias, no a la verdad global.

---

## 14. Inferencia

La inferencia produce hipótesis, no verdades.

Las reglas son datos declarativos y portables. El lenguaje:
- no es Turing-completo;
- no ejecuta código arbitrario;
- no accede a red/filesystem;
- tiene profundidad y presupuesto máximos;
- limita hipótesis nuevas;
- garantiza terminación bajo cuotas.

---

## 15. Actualización y sesgos

La evidencia nueva no sustituye automáticamente una creencia.

Influyen:
- certeza previa;
- carga emocional;
- confianza y respeto hacia la fuente;
- claridad perceptiva;
- rigidez derivada;
- curiosidad;
- emociones actuales;
- coherencia con otras creencias;
- independencia de fuentes.

Esto permite sesgo de confirmación sin reglas específicas por personaje.

---

## 16. Appraisal

La evaluación subjetiva considera:
- relevancia;
- consecuencia;
- agencia;
- intención percibida;
- certeza;
- control;
- norma;
- relación;
- expectativa previa.

La novedad deriva preferentemente del error de predicción entre expectativa y resultado.

---

## 17. Emociones

Seis emociones base candidatas:
- alegría;
- tristeza;
- miedo;
- ira;
- ansiedad;
- aversión.

Cada emoción es una instancia con intensidad, objetivo, causa percibida, creencia asociada, timestamp y decaimiento.

Estados derivados como orgullo, culpa, gratitud, duelo, resentimiento, alivio o esperanza se calculan a partir del estado existente y no requieren barras independientes.

---

## 18. Ánimo y reapreciación

El ánimo usa:
- valencia;
- activación.

La reapreciación permite cambiar la carga emocional de una memoria sin cambiar necesariamente su contenido factual.

---

## 19. Relaciones

Relaciones direccionales:
- afecto;
- confianza;
- respeto;
- familiaridad.

`A → B` y `B → A` son estados independientes.

---

## 20. Memoria

Una memoria conserva:
- proposición/evento principal;
- interpretación;
- emoción original;
- importancia;
- fuente;
- certeza;
- participantes;
- timestamp;
- reactivaciones;
- referencias externas.

La consolidación depende de intensidad, relevancia, recurrencia y consecuencias prolongadas.

---

## 21. Aprendizaje causal inicial

Primera versión deliberadamente acotada:

```text
ActionType + ContextTags -> OutcomeTags
```

Cada patrón puede mantener intentos, éxitos, frecuencia de resultados, confianza y última activación.

No se intenta aprendizaje simbólico abierto en la primera versión.

---

## 22. Exploración y explotación

Curiosidad modula la tolerancia a incertidumbre y la preferencia por probar alternativas frente a repetir estrategias conocidas.

---

## 23. Regulación emocional

Una emoción intensa puede generar metas regulatorias o modificar utilidad de acciones. La regulación no sustituye al sistema de decisión.

---

## 24. Emoción sentida y expresada

El estado interno y la expresión observable son distintos. La expresión depende de personalidad, contexto, normas, relaciones y metas.

Otros agentes reaccionan a señales observables entregadas por el host, no al estado interno oculto.

---

## 25. Normas y cultura

Las normas pueden representarse como creencias y valores individuales sobre lo aceptable y sobre lo que el grupo espera.

La cultura no es un controlador global. Se interpreta como distribución de valores, creencias, expectativas y prácticas entre agentes.

Métricas poblacionales pueden calcularse externamente leyendo estados individuales.

---

## 26. Comunicación y transmisión

Una interacción social puede transmitir proposiciones. El receptor evalúa la afirmación según fuente, certeza, relaciones, evidencia previa y sesgos.

No se requiere un sistema global de rumores para transmitir creencias.

---

## 27. Influencia externa

MSPA puede recibir fuentes externas de influencia como estímulos semánticos. El agente puede aceptar, reinterpretar, rechazar o ignorar según su estado.

La influencia nunca equivale automáticamente a una orden ejecutada.

---

## 28. Portabilidad

Los estados psicológicos pueden contener referencias a entidades o conceptos no resolubles en el runtime actual. Esas referencias pueden preservarse de forma opaca.

Los namespaces evitan colisiones entre orígenes distintos.

---

## 29. Decisión

El host ofrece acciones posibles. MSPA las evalúa mediante consideraciones relevantes.

La personalidad modifica pesos y sensibilidades, no suma bonificaciones rígidas.

Consideraciones típicas:
- necesidad;
- meta;
- valor;
- emoción;
- relación;
- memoria;
- norma;
- contexto;
- coste;
- riesgo;
- expectativa aprendida.

No todas se calculan para todas las acciones.

---

## 30. Curvas y composición de utilidad

Cada consideración transforma entradas normalizadas mediante curvas configurables y serializables, inicialmente lineales por tramos.

La composición distingue:
- contribuciones positivas/negativas;
- condicionantes;
- restricciones físicas ya resueltas por el host;
- histéresis;
- ruido determinista pequeño.

La fórmula final se valida por conformidad antes de congelarse.

---

## 31. Histéresis

Una acción en curso recibe continuidad mediante `ActionContinuityKey`, progreso y coste de interrupción.

Una alternativa debe superar suficientemente la preferencia actual salvo urgencia crítica.

---

## 32. Tiempo y evaluación perezosa

`PsychTime` usa ticks enteros de 64 bits.

Emociones, ánimo, memoria y otros decaimientos se actualizan de forma perezosa a partir de timestamps cuando sea posible.

No se requiere actualizar todos los agentes cada frame.

---

## 33. Batch y determinismo

Toda operación individual puede tener forma batch equivalente.

Reglas:
- orden canónico estable;
- RNG independiente por agente;
- estructuras compartidas de sólo lectura durante el batch;
- resultado independiente del scheduling de threads.

---

## 34. Seguridad

Todo estado, pack o regla externa se considera no confiable.

Se aplican cuotas a:
- tamaño total;
- número de creencias;
- memorias;
- metas;
- relaciones;
- reglas;
- profundidad de proposiciones;
- strings;
- patrones de aprendizaje;
- derivaciones.

Jerarquía:
1. hard limits MSPA;
2. runtime policy del host;
3. recomendaciones externas.

---

## 35. Domain Packs

MSPA define el contrato de Domain Pack, pero los packs concretos pertenecen a proyectos consumidores externos.

Pueden declarar vocabulario, Predicate Schemas, Event Templates, Action Schemas, reglas de inferencia, consideraciones, curvas y tags de aprendizaje.

Ningún Domain Pack concreto forma parte del diseño normativo de MSPA.

---

## 36. Explicabilidad

MSPA debe poder explicar:
- qué percibió;
- qué creencia cambió;
- qué inferencia se produjo;
- qué memoria se activó;
- qué emociones influyeron;
- qué acciones se consideraron;
- qué factores determinaron la decisión.

La instrumentación no altera la simulación.

---

## 37. Conformidad

Antes de congelar una API de producción se validan escenarios **abstractos**, sin nombres ni contenido de proyectos consumidores, que cubran:
- divergencia de creencias;
- contradicción de evidencia;
- reapreciación;
- influencia externa;
- referencias no resolubles;
- aprendizaje causal simple;
- histéresis;
- batch vs individual;
- determinismo;
- seguridad ante entradas hostiles.

---

## 38. Invariante de separación

**MSPA no incorpora personajes, entidades, lugares, eventos, namespaces, nombres de repositorios ni reglas de un host concreto en su diseño normativo ni en su core.**

Todo elemento específico pertenece al proyecto consumidor o a datos externos compatibles con los contratos públicos.