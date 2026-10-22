# MSPA Interaction Contract v0.2

## Estado

Contrato lógico candidato. No es todavía una API de producción ni define sintaxis concreta, lenguaje, ABI o formato de serialización.

Este documento fija qué intercambian el host y MSPA y qué garantías lógicas deben conservarse entre implementaciones.

---

# 1. Operaciones fundamentales

MSPA se organiza alrededor de cinco operaciones conceptuales:

1. `OBSERVE`
2. `DECIDE`
3. `REPORT`
4. `ADVANCE_TO`
5. `EXPORT / IMPORT`

Todas las operaciones relevantes llevan tiempo lógico explícito.

---

# 2. OBSERVE

## Entrada conceptual

`OBSERVE(agent, stimulus, time)`

El host entrega sólo información a la que el agente tuvo acceso.

Un estímulo puede representar:

- evento físico;
- testimonio;
- resultado observado de otro;
- estado interno;
- cambio ambiental;
- inspiración o influencia;
- comunicación social;
- recordatorio contextual.

## Stimulus

Campos conceptuales:

- `StimulusId`
- `StimulusType`
- `source`
- `target`
- `propositions[]`
- `perceptual_confidence`
- `salience`
- `namespace`
- `timestamp`
- metadatos opcionales del Domain Pack

## Salida conceptual

`OBSERVE` no selecciona una acción, pero devuelve información de scheduling:

- `state_changed`
- `decision_recommended`
- `urgency`
- `current_action_threatened`
- `diagnostic_token` opcional

Esto permite que el host llame inmediatamente a `DECIDE` cuando el estímulo vuelve obsoleta o peligrosa la conducta actual.

---

# 3. DECIDE

## Entrada conceptual

`DECIDE(agent, available_actions, current_action?, time)`

Puede llamarse sin un `OBSERVE` inmediatamente anterior.

## ActionOption

Cada opción debe contener suficiente semántica para que MSPA pueda valorarla:

- `ActionTypeId`
- `ActionInstanceId`
- `ActionContinuityKey`
- actor
- objetivo(s)
- Predicate Schema o semántica registrada
- efectos esperados
- costes
- riesgos
- tags
- consideraciones registradas
- restricciones ya resueltas por el host
- metadatos de progreso si representa una acción en curso

### Identidad estable

`ActionInstanceId` puede ser efímero.

`ActionContinuityKey` debe permanecer estable mientras la opción represente la misma actividad lógica.

Ejemplo:

> “seguir construyendo la misma casa” debe conservar continuidad aunque el host regenere la lista de opciones cada ciclo.

## Salida conceptual

`Decision`:

- `selected_action`
- `selected_target`
- `confidence`
- `intention`
- `expressed_emotion`
- `outgoing_statements[]`
- `continuity_expectation`
- `explanation`

MSPA sólo puede seleccionar entre las acciones ofrecidas por el host.

---

# 4. REPORT

## Entrada conceptual

`REPORT(agent, action_result, time)`

`ActionResult` puede incluir:

- acción intentada;
- continuidad asociada;
- éxito / fracaso / parcial;
- resultado observable;
- OutcomeTags;
- consecuencias inesperadas;
- terceros afectados;
- motivo de interrupción;
- costes reales;
- cambios de recursos;
- proposiciones resultantes.

## Efectos lógicos

MSPA puede usar el reporte para:

- cerrar o modificar metas;
- actualizar emociones;
- reforzar o debilitar expectativas;
- registrar aprendizaje causal;
- modificar relaciones;
- formar memoria;
- generar nuevas creencias.

---

# 5. ADVANCE_TO

`ADVANCE_TO(agent, target_time)`

No es un tick obligatorio.

Su uso principal es:

- actualizar agentes dormidos;
- caducar metas;
- resolver hitos temporales;
- materializar efectos perezosos cuando sea necesario;
- informar al host si el paso del tiempo por sí solo recomienda una nueva decisión.

Todas las demás operaciones ya transportan timestamps, por lo que un host puede usar `ADVANCE_TO` sólo cuando lo necesite.

---

# 6. EXPORT / IMPORT

## EXPORT

Produce un `AgentPsychState` portable y versionado.

Debe incluir todo lo necesario para reconstruir el agente psicológico sin depender de punteros, memoria local o IDs efímeros del runtime.

## IMPORT

Importa un estado después de:

1. validar versión;
2. validar estructura;
3. validar namespaces y packs;
4. validar límites de tamaño/complejidad;
5. resolver referencias conocidas;
6. preservar referencias externas desconocidas;
7. rechazar reglas o datos no autorizados.

La autenticación del jugador/servidor pertenece al host. La seguridad estructural del estado pertenece a MSPA.

---

# 7. AgentPsychState

Conceptualmente debe poder contener:

- identidad psicológica del agente;
- personalidad;
- valores;
- metas;
- creencias;
- emociones activas;
- ánimo;
- relaciones;
- memorias;
- expectativas aprendidas;
- referencias externas;
- RNG/seed o información suficiente para reproducibilidad;
- versión de estado;
- packs y namespaces requeridos.

No se fija todavía formato de serialización.

---

# 8. Proposition

Una proposición contiene:

- `PredicateId`
- roles y argumentos
- polaridad
- tiempo/intervalo
- calificadores
- namespace
- procedencia

MSPA debe poder comparar proposiciones al menos para detectar:

- igualdad semántica básica;
- contradicción;
- compatibilidad;
- relación de evidencia cuando una regla lo declara.

---

# 9. Predicate Schema

Todo predicado de dominio que MSPA deba interpretar debe estar registrado.

El esquema declara:

- roles válidos;
- rol agente;
- posibles objetivos;
- categorías de efecto;
- información de intencionalidad;
- relevancias posibles;
- tags de aprendizaje;
- pistas de appraisal;
- mapeos opcionales hacia vocabulario base MSPA.

Un predicado desconocido puede conservarse como dato opaco, pero no puede recibir interpretación psicológica rica sin esquema.

---

# 10. Reglas de inferencia

Las reglas son datos declarativos.

Deben poder serializarse/versionarse y no pueden ejecutar código arbitrario.

El runtime debe imponer:

- profundidad máxima;
- número máximo de reglas por estímulo;
- máximo de hipótesis nuevas;
- presupuesto temporal o lógico;
- terminación garantizada.

Las reglas pueden producir hipótesis con una certeza base que luego se combina con la calidad de la evidencia y los sesgos del agente.

---

# 11. Consideraciones de utilidad

Cada tipo de acción declara sólo las consideraciones relevantes.

Cada consideración obtiene una entrada normalizada y la transforma mediante una curva serializable, preferiblemente lineal por tramos.

Una implementación debe distinguir:

- contribuciones de utilidad;
- factores de penalización fuerte;
- restricciones físicas ya eliminadas por el host.

No se fija todavía una única fórmula final de combinación; el sandbox debe comparar alternativas y verificar estabilidad.

---

# 12. Histéresis

La decisión debe poder reconocer una acción actual mediante `ActionContinuityKey`.

La continuidad puede considerar:

- progreso;
- coste de interrupción;
- persistencia de meta;
- compromiso;
- urgencia nueva.

Un host que no proporcione continuidad estable acepta una degradación explícita: MSPA no puede garantizar histéresis robusta.

---

# 13. Batch API conceptual

Toda operación individual debe admitir una forma batch equivalente:

- `OBSERVE_BATCH`
- `DECIDE_BATCH`
- `REPORT_BATCH`
- `ADVANCE_BATCH`

No se fija todavía nombre real de API.

Garantía:

> procesar un batch debe ser semánticamente equivalente a procesar sus agentes individualmente en orden canónico.

El orden canónico debe estar definido por referencias estables, no por hash maps ni por orden de threads.

---

# 14. Determinismo

Cada agente debe disponer de un stream pseudoaleatorio independiente o derivable de forma estable.

Agregar un agente nuevo no debe alterar la secuencia aleatoria de los demás.

No se permite depender de:

- orden de iteración no especificado;
- scheduling de threads;
- RNG global compartido;
- funciones matemáticas cuya variación multiplataforma comprometa la suite de conformidad sin una política explícita.

---

# 15. Tiempo

Toda llamada relevante lleva timestamp lógico.

Los decaimientos deben poder evaluarse perezosamente.

Una implementación no debe requerir actualizar todos los agentes cada frame para mantener coherencia.

---

# 16. Aprendizaje causal inicial

La primera versión usa patrones acotados:

```text
ActionType + ContextTags -> OutcomeTags
```

Cada patrón conserva contadores/estadísticas limitadas.

El Domain Pack controla qué ContextTags son relevantes para evitar explosión combinatoria.

Los límites máximos de patrones por agente forman parte de la configuración de seguridad/rendimiento.

---

# 17. Entrada no confiable

Los estados y packs externos deben tratarse como datos hostiles.

El runtime debe imponer cuotas a:

- tamaño total;
- número de creencias;
- memorias;
- metas;
- relaciones;
- reglas;
- profundidad de proposiciones;
- longitud de strings;
- patrones de aprendizaje;
- derivaciones máximas.

No basta con validar sintaxis.

---

# 18. Compatibilidad de packs

Cada pack declara:

- `pack_id`
- versión
- namespace
- dependencias
- compatibilidad
- vocabulario
- schemas
- reglas
- curvas
- límites

Durante importación, conceptos desconocidos pueden permanecer opacos y referencias externas pueden seguir colgantes.

---

# 19. Reglas de degradación

El comportamiento debe ser definido cuando módulos opcionales estén apagados.

Ejemplos:

- sin inferencia: sólo percepción/testimonio directo;
- sin memoria: no hay reactivación episódica;
- sin aprendizaje: no se actualizan expectativas causales;
- sin expresión: `expressed_emotion` y `outgoing_statements` pueden quedar vacíos;
- sin relaciones: appraisal social usa valores neutrales configurables;
- sin ánimo: emociones operan directamente.

---

# 20. Escenarios de conformidad

Este contrato no se congela hasta expresar y ejecutar como datos:

1. Mara;
2. inspiración;
3. viajero entre mundos;
4. histéresis;
5. batch vs individual;
6. determinismo;
7. estado hostil;
8. aprendizaje causal simple;
9. contacto cultural sin regla especial.

La suite de escenarios debe sobrevivir al reemplazo del sandbox por la implementación de producción.
