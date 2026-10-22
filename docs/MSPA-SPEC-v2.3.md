# MSPA-Lib — Especificación lógica v2.3 (arquitectura candidata)

## 1. Estado del documento

MSPA-Lib es un **Motor de Simulación Psicológica y Cultural de Agentes** diseñado como middleware reutilizable para juegos y simulaciones.

Esta versión es una **arquitectura candidata**, no una especificación congelada. Antes de congelar el núcleo deben demostrarse en un sandbox los mecanismos de creencias, inferencia, utilidad, aprendizaje, determinismo y transmisión social.

No existe implementación de producción asociada a este documento.

---

## 2. Propósito

MSPA recibe estímulos y contexto desde un proyecto anfitrión, mantiene estado psicológico persistente por agente y devuelve preferencias de conducta explicables.

MSPA procesa significado psicológico. El host controla la realidad y la ejecución.

El host conserva autoridad sobre movimiento, navegación, combate, inventarios, físicas, animación, escenas, recursos, red, UI y toda regla concreta del mundo.

MSPA no debe conocer conceptos específicos como isla, trigo, muelle, espada, hechizo o nave espacial salvo a través de vocabulario y paquetes de dominio registrados por el host.

---

## 3. Contrato fundamental

El ciclo lógico se divide en operaciones independientes:

1. **OBSERVE** — el host entrega un estímulo percibido por un agente.
2. MSPA actualiza creencias, emociones, relaciones, memoria, metas y aprendizaje cuando corresponda.
3. **DECIDE** — el host entrega las acciones realmente disponibles.
4. MSPA devuelve una intención elegida entre esas opciones.
5. El host ejecuta o intenta ejecutar la acción.
6. **REPORT** — el host informa el resultado real.
7. MSPA aprende y actualiza el estado.
8. **ADVANCE** — el host comunica paso de tiempo lógico cuando sea necesario.
9. **EXPORT / IMPORT** — el estado psicológico puede persistirse o viajar entre runtimes.

`REACT(estímulo, opciones)` puede existir en una implementación futura como conveniencia equivalente a OBSERVE + DECIDE, pero no es la abstracción fundamental.

---

# PARTE I — IDENTIDAD Y MOTIVACIÓN

## 4. Personalidad

Se mantienen seis ejes base relativamente estables:

| Eje | Bajo | Alto | Función |
|---|---|---|---|
| Sociabilidad | reservado | sociable | peso de interacción y compañía |
| Estabilidad | reactivo | sereno | sensibilidad, intensidad y recuperación emocional |
| Empatía | egocéntrico | empático | relevancia del bienestar ajeno |
| Persistencia | flexible | obstinado | constancia general, nunca efecto único universal |
| Curiosidad | tradicional | curioso | búsqueda de información, exploración e incertidumbre |
| Audacia | prudente | temerario | peso subjetivo del riesgo |

### 4.1 Persistencia no es un interruptor universal

Persistencia no debe controlar directamente todos los fenómenos de obstinación. De ella se derivan efectos distintos combinándola con otras variables.

Ejemplos conceptuales:

- **persistencia de meta**: Persistencia + Estabilidad + inversión previa;
- **rigidez de creencia**: Persistencia - Curiosidad + carga emocional + identidad/valor amenazado;
- **resistencia a interrumpir una acción**: Persistencia + progreso + coste de interrupción.

Esto evita que un solo eje fabrique por sí mismo los resultados esperados.

### 4.2 Regla central

La personalidad modifica sensibilidad, valoración y curvas de respuesta; no aplica bonificaciones rígidas a acciones concretas.

---

## 5. Valores

Los valores indican qué considera importante un agente. El catálogo es extensible por dominio.

MSPA sólo necesita identidad, intensidad, relaciones de compatibilidad/conflicto y metadatos semánticos suficientes.

Ejemplos de un dominio: familia, comunidad, prosperidad, conocimiento, fe, autonomía, prestigio.

Los valores no son rasgos de personalidad.

---

## 6. Necesidades

Las necesidades pertenecen principalmente al host. MSPA recibe su estado psicológico relevante.

Una necesidad no es una emoción. Hambre, sed o dolor pueden influir en emociones y decisiones sin convertirse en emociones.

---

## 7. Metas activas

Una meta contiene conceptualmente:

- origen;
- condición objetivo;
- prioridad;
- estado;
- condición de cierre;
- contexto temporal;
- posibilidad de suspensión o abandono.

Orígenes posibles:

- necesidad;
- valor;
- compromiso;
- evento;
- creencia;
- relación;
- regulación emocional;
- aprendizaje.

Los compromisos son un origen de meta, no un subsistema independiente.

El diseño debe favorecer pocas metas dominantes simultáneas. Como referencia inicial: aproximadamente 1–3 metas principales por agente, con otras suspendidas o latentes.

---

# PARTE II — LENGUAJE SEMÁNTICO Y CREENCIAS

## 8. Proposiciones

Las creencias no pueden ser texto libre opaco. MSPA necesita una representación semántica mínima.

Una proposición contiene conceptualmente:

- predicado;
- argumentos con roles;
- polaridad;
- marco temporal;
- calificadores opcionales;
- procedencia/namespace.

Ejemplo:

```text
predicado: mspa:present_at
actor: Torvando
lugar: Casa_Mara
momento: Incendio_17
polaridad: true
```

Otro ejemplo:

```text
predicado: mspa:failed_to_prevent
actor: Torvando
evento: muerte_Mara
intencion: unknown
```

Este esquema permite detectar compatibilidad, contradicción, entidades compartidas y evidencia relacionada.

---

## 9. Vocabulario base MSPA

Para asegurar interoperabilidad, MSPA define un vocabulario semántico base común. Los juegos pueden extenderlo mediante namespaces propios.

El vocabulario base debe cubrir relaciones psicológicamente generales, por ejemplo:

- `mspa:present_at`
- `mspa:caused`
- `mspa:helped`
- `mspa:harmed`
- `mspa:prevented`
- `mspa:failed_to_prevent`
- `mspa:able_to_help`
- `mspa:owns`
- `mspa:belongs_to`
- `mspa:promised`
- `mspa:threatens`
- `mspa:expects`
- `mspa:desires`
- `mspa:approves`
- `mspa:forbids`
- `mspa:communicated`

Este catálogo no está cerrado todavía; debe validarse antes del freeze.

Un dominio puede añadir `isla:harvested`, `rpg:cast_spell`, etc. sin modificar el núcleo.

---

## 10. Fuentes epistemológicas

Una creencia puede originarse en:

- observación directa;
- testimonio;
- inferencia;
- recuerdo;
- inspiración/influencia;
- aprendizaje causal;
- combinación de creencias previas.

Las fuentes no tienen el mismo peso.

---

## 11. Creencias

Una creencia contiene:

- proposición;
- certeza;
- fuente;
- origen;
- carga emocional asociada;
- última activación;
- evidencias principales;
- entidades implicadas;
- namespace/procedencia.

MSPA reacciona a lo que el agente cree, no a la verdad global del mundo.

---

## 12. Inferencia

MSPA no inventa semántica específica del mundo. Una inferencia debe derivarse de reglas declarativas.

Forma conceptual:

```text
SI
    A mspa:present_at E
    B mspa:in_danger E
    A mspa:able_to_help B
    A mspa:left E
ENTONCES
    hipotesis: A mspa:failed_to_help B
    certeza_base: media
```

Las reglas producen **hipótesis**, no verdad.

La misma observación puede generar una hipótesis con certeza moderada mientras otras creencias directas mantienen certeza alta.

---

## 13. Reglas de inferencia como datos

Las reglas de inferencia deben ser declarativas, serializables, versionables y asociadas a un paquete de contenido.

No deben depender exclusivamente de callbacks de código del host si se pretende portabilidad entre mundos o multiplayer.

Una implementación futura podrá compilar/optimizar estas reglas, pero su definición lógica debe existir como datos.

---

## 14. Actualización y sesgo de confirmación

La evidencia nueva no reemplaza automáticamente una creencia.

Influyen:

- certeza previa;
- carga emocional;
- confianza y respeto por la fuente;
- familiaridad;
- claridad perceptiva;
- rigidez derivada;
- Curiosidad;
- emoción activa;
- coherencia con otras creencias;
- amenaza a valores/metas importantes.

La misma evidencia puede producir conclusiones distintas en agentes distintos.

Una creencia no aumenta su certeza por mero paso del tiempo. Puede fortalecerse por repetición, reactivación, evidencia compatible o convergencia de fuentes; puede debilitarse por olvido, contradicción, pérdida de confianza o reapreciación.

---

## 15. Reapreciación

Actualizar una creencia y reapreciar una memoria son procesos diferentes.

Un agente puede mantener:

> “Torvando falló.”

pero pasar de ira intensa a aceptación o perdón sin cambiar el hecho que cree verdadero.

Al reactivar una memoria, MSPA puede recalcular parcialmente su carga emocional usando estado actual, relación presente y memorias relacionadas.

---

# PARTE III — PERCEPCIÓN, ATENCIÓN Y EXPECTATIVAS

## 16. Percepción

El host determina qué pudo percibir el agente y con qué claridad. MSPA no calcula línea de visión, distancia acústica ni física del mundo salvo datos proporcionados por el host.

---

## 17. Atención

Antes de evaluación psicológica se filtra por:

- acceso perceptivo;
- relevancia para metas, valores, necesidades o relaciones;
- saliencia;
- presupuesto de procesamiento del nivel de simulación.

Un evento irrelevante puede no producir creencia, emoción ni memoria.

---

## 18. Expectativas y novedad

La novedad no requiere un predictor separado si puede derivarse de expectativas aprendidas.

Si un agente espera que:

> X + contexto C → Y

pero observa:

> X + contexto C → Z

existe un **error de predicción**.

Ese error puede aumentar curiosidad, ansiedad, revisión de creencias o exploración.

Si el host no dispone de expectativa aplicable, `unexpectedness` puede proporcionarse como metadato opcional.

---

# PARTE IV — APRENDIZAJE

## 19. Aprendizaje causal

MSPA representa aprendizaje práctico mediante creencias causales del tipo:

```text
accion X + contexto C -> resultado Y
```

La certeza aumenta o disminuye según experiencia acumulada.

No se requiere un `LearningSystem` independiente si el mecanismo se expresa con el sistema de creencias.

---

## 20. Abstracción del contexto

Las experiencias no deben memorizar cada detalle irrelevante. El Domain Pack define rasgos/contextos generalizables.

Ejemplo:

```text
accion: talar
contexto:
    bosque
    herramienta_adecuada
    dia
resultado:
    madera
```

MSPA debe permitir generalización y especialización de reglas sin generar una creencia distinta por cada coordenada, objeto o instante.

El diseño final debe definir:

- límite de complejidad contextual;
- compactación;
- olvido;
- especialización cuando una regla falla sistemáticamente;
- fusión de experiencias compatibles.

---

## 21. Exploración y explotación

Curiosidad modifica la preferencia entre:

- **explotar** una conducta conocida con buen resultado esperado;
- **explorar** alternativas de mayor incertidumbre.

No se fija todavía una fórmula definitiva. Debe probarse en sandbox.

---

## 22. Aprendizaje social

Un agente puede aprender observando o recibiendo información de otros.

El peso depende de:

- claridad de observación;
- confianza;
- respeto;
- familiaridad;
- prestigio percibido;
- compatibilidad con creencias existentes.

La imitación es una consecuencia posible, no una orden global.

---

# PARTE V — EMOCIONES Y ÁNIMO

## 23. Appraisal

Un estímulo no produce emociones directamente. MSPA evalúa lo que el agente cree que ocurrió.

Factores principales:

- relevancia;
- consecuencia;
- agencia;
- intención percibida;
- certeza;
- control;
- expectativa/novedad;
- norma personal/social percibida;
- relación con implicados.

Las plantillas de evento pueden predefinir elementos no subjetivos; el agente calcula lo dependiente de su estado.

---

## 24. Emociones básicas

Se mantienen como hipótesis de diseño seis emociones fundamentales:

- Alegría
- Tristeza
- Miedo
- Ira
- Ansiedad
- Aversión

La distribución 1 positiva / 5 negativas no se considera todavía un defecto demostrado. Debe validarse midiendo la valencia de poblaciones sanas durante simulaciones largas.

Emociones derivadas como orgullo, gratitud, culpa, vergüenza, celos, duelo, resentimiento, alivio o esperanza no necesitan barras independientes.

---

## 25. Emociones como instancias

Cada emoción puede tener:

- tipo;
- intensidad;
- objetivo;
- causa percibida;
- creencia/memoria asociada;
- tiempo de origen;
- decaimiento.

El decaimiento debe favorecer evaluación perezosa mediante marcas de tiempo, no actualización por frame.

---

## 26. Regulación y expresión

Sentir y mostrar son distintos.

Una emoción puede producir estrategias como:

- confrontar;
- evitar;
- buscar apoyo;
- aislarse;
- distraerse;
- rumiar;
- reconciliarse;
- rezar;
- solicitar información.

La expresión observable puede diferir del estado interno según personalidad, normas, metas y contexto.

---

## 27. Ánimo

El ánimo usa dos dimensiones internas:

- valencia;
- activación.

Debe tener una línea basal y retornar lentamente hacia ella salvo condiciones persistentes.

No debe confundirse activación psicológica con energía física.

---

# PARTE VI — MEMORIA Y RELACIONES

## 28. Memoria

Una memoria significativa contiene:

- proposiciones/evento recordado;
- interpretación;
- emoción original;
- importancia;
- fuente;
- certeza;
- fecha;
- participantes;
- claves de reactivación.

Las memorias pueden ser triviales, consolidadas o vitales.

La consolidación depende de intensidad, relevancia, recurrencia y consecuencias prolongadas; no sólo de repetición.

---

## 29. Relaciones

Las relaciones interpersonales base son direccionales:

- afecto;
- confianza;
- respeto;
- familiaridad.

A → B no implica B → A.

Deuda social no es una dimensión obligatoria; puede representarse mediante memoria, gratitud y metas/compromisos.

MSPA debe poder extender la relación hacia entidades sociales abstractas cuando un dominio lo necesite: institución, grupo, figura divina percibida, etc.

---

# PARTE VII — CULTURA EMERGENTE

## 30. Cultura como distribución

MSPA no necesita una entidad central que ordene la cultura.

La cultura puede observarse como distribución poblacional de:

- valores;
- creencias normativas;
- expectativas;
- prácticas aprendidas;
- símbolos/conceptos compartidos.

`CultureId` puede servir como etiqueta estadística o de procedencia, no como fuente omnisciente de conducta.

---

## 31. Norma personal y norma social percibida

Se distinguen:

- **convicción personal**: “creo que X está bien/mal”;
- **norma social percibida**: “creo que mi grupo espera X”.

Esto permite conformidad, disidencia, hipocresía, reforma y cambio cultural sin clases especiales.

---

## 32. Enculturación

Niños y nuevos miembros pueden adquirir valores, normas, expectativas y prácticas por:

- cuidadores;
- observación;
- pares;
- figuras respetadas;
- experiencias directas.

La enculturación modifica probabilidades y creencias, no crea clones culturales deterministas.

---

## 33. Conformidad, prestigio e innovación

La conformidad debe emerger de creencias sobre expectativas del grupo y costes sociales percibidos.

Prestigio puede derivarse de respeto agregado/percibido y aumentar el peso del aprendizaje social.

Innovación puede emerger de exploración y aprendizaje: probar una variante, obtener un resultado, repetirla, ser imitado y eventualmente convertirla en práctica compartida.

Cadena hipotética:

```text
experimento -> aprendizaje -> hábito -> imitación -> práctica compartida -> norma/costumbre
```

Esto es una **hipótesis a validar**, no una promesa de que toda simulación generará culturas interesantes o religiones emergentes.

---

## 34. Transmisión social

MSPA no necesita un sistema global de rumores.

Una afirmación transmitida por A se convierte en estímulo para B. El host realiza la interacción observable; MSPA procesa la creencia testimonial.

La transmisión puede introducir pérdida de certeza, simplificación, interpretación o distorsión.

La reputación pública compleja puede posponerse, pero deben existir mecanismos suficientes para que creencias sobre terceros circulen.

---

# PARTE VIII — INFLUENCIA EXTERNA / GOD-GAME

## 35. Fuentes de influencia

MSPA no codifica “Dios” como concepto universal. Define una fuente de influencia semántica que un dominio puede usar como voz divina, IA central, propaganda, magia, implante, etc.

Una inspiración es un estímulo, no una orden absoluta.

Ejemplo:

```text
fuente: divine_player_01
contenido: protect(Mara)
intensidad: alta
```

El agente puede aceptarla, reinterpretarla, resistirla o ignorarla según sus creencias, valores, personalidad y relación con la fuente percibida.

Esto puede permitir que creencias religiosas emerjan, pero debe validarse experimentalmente.

---

# PARTE IX — DECISIÓN

## 36. Opciones proporcionadas por el host

MSPA nunca inventa una acción ejecutable fuera del espacio ofrecido por el host.

El host entrega `ActionOption` con suficiente semántica para que MSPA la evalúe.

MSPA puede mantener metas no ejecutables todavía, pero no puede ejecutar acciones inexistentes.

---

## 37. Consideraciones de acción

Cada acción evalúa sólo las consideraciones relevantes.

Ejemplos:

- necesidad;
- meta;
- valor;
- emoción;
- relación;
- memoria;
- norma;
- coste;
- riesgo;
- expectativa de resultado;
- contexto.

No todas las acciones deben sumar todos los factores.

Cada consideración transforma entradas normalizadas mediante curvas de respuesta configurables.

---

## 38. Composición de utilidad

La arquitectura candidata permite dos categorías:

1. **contribuciones** que aumentan o reducen utilidad;
2. **condicionantes** que reducen fuertemente la viabilidad psicológica sin ser necesariamente prohibiciones físicas.

Las restricciones físicas/reales pertenecen al host y eliminan acciones antes de MSPA.

Normas y tabúes psicológicos normalmente deben ser costes fuertes, no prohibiciones absolutas, salvo reglas deliberadas del diseño del juego.

La fórmula numérica definitiva todavía no está congelada. Debe probarse mediante sandbox y curvas de respuesta.

---

## 39. Histéresis

Una acción en curso recibe compromiso temporal. MSPA evita cambiar de acción por pequeñas fluctuaciones.

Una acción puede interrumpirse si:

- termina;
- aparece una emergencia;
- deja de ser viable;
- otra opción supera claramente el margen de cambio;
- una necesidad crítica obliga a reconsiderar.

---

## 40. Ruido

El ruido decisional debe ser pequeño, controlado y reproducible.

Su función es resolver empates y evitar conducta perfectamente mecánica, no sustituir causalidad.

---

# PARTE X — ESCALA Y TIEMPO

## 41. Event-driven y lazy evaluation

MSPA debe diseñarse para evitar un “tick psicológico completo” por frame.

Emociones, ánimo, memorias, relaciones y certeza pueden almacenar marcas de tiempo y calcular decaimiento al consultarse o al recibir eventos relevantes.

---

## 42. Niveles de simulación

El host puede asignar niveles conceptuales:

- **Activo**: evaluación completa y frecuente;
- **Reducido**: sólo eventos relevantes y decisiones espaciadas;
- **Dormido**: actualización diferida de largo plazo.

Al reactivarse un agente, MSPA actualiza su estado desde marcas temporales.

No se promete todavía una cifra concreta de agentes simultáneos; el rendimiento debe medirse.

---

## 43. Escalas temporales

Referencias lógicas:

| Capa | Escala típica |
|---|---|
| emoción | segundos a días |
| regulación | minutos a días |
| ánimo | días |
| metas inmediatas | minutos a días |
| metas personales | días a años |
| relaciones | días a años |
| memoria | semanas a toda la vida |
| personalidad | años |

Estas escalas son configurables por dominio.

---

# PARTE XI — PORTABILIDAD Y MULTIPLAYER

## 44. Identidad global y namespaces

Los identificadores locales no bastan cuando agentes viajan entre mundos.

Toda referencia portable debe poder distinguir al menos:

```text
World/Namespace + EntityId
```

Dos entidades con el mismo ID local en mundos distintos no son la misma entidad.

---

## 45. Referencias colgantes

Un agente importado puede recordar entidades o lugares inexistentes en el nuevo host.

MSPA debe conservar referencias externas con metadatos mínimos suficientes para mantener el significado psicológico aunque no puedan resolverse localmente.

Migrar un agente no debe borrar recuerdos de su mundo de origen.

---

## 46. Conceptos compartidos y desconocidos

Los conceptos pueden ser:

- base MSPA;
- compartidos por packs compatibles;
- específicos del mundo de origen;
- desconocidos para el receptor.

Un concepto desconocido puede mantenerse opaco y transportable hasta que exista una traducción, observación o mapeo.

La falta de expectativa sobre una práctica extranjera puede generar incertidumbre/novedad de forma natural.

---

## 47. Export / Import

El runtime puede ser propietario del estado en ejecución por rendimiento, pero debe poder exportar/importar un `AgentPsychState` autocontenido.

Un viajero puede trasladarse así:

```text
export agent state -> transport -> validate -> import
```

El estado portable debe conservar personalidad, valores, metas relevantes, creencias, memoria, relaciones externas, aprendizaje y procedencia.

---

## 48. Validación

MSPA debe distinguir validación estructural de autenticidad de red.

El núcleo valida:

- versión;
- rangos;
- referencias;
- namespaces;
- dependencias de pack;
- estructura de reglas y proposiciones.

Autenticación, firma y confianza de red pertenecen principalmente al host.

---

# PARTE XII — CONTENT PACKS

## 49. Core Psychology Pack

MSPA debe incluir un paquete base razonable para evitar ser una librería vacía.

Puede contener:

- vocabulario base;
- rasgos base;
- emociones base;
- appraisal genérico;
- relaciones base;
- reglas sociales/inferencias generales cuidadosamente limitadas;
- curvas iniciales de utilidad;
- degradaciones por defecto.

---

## 50. Domain Pack

El coste real de integración de un juego reside en su paquete de dominio.

Un Domain Pack puede definir:

- namespace;
- predicados específicos;
- plantillas de estímulo/evento;
- reglas declarativas de inferencia;
- categorías de acción;
- curvas de consideración;
- etiquetas de contexto para aprendizaje;
- conceptos exportables;
- parámetros temporales;
- normas iniciales opcionales;
- compatibilidad y migraciones.

Debe tener:

- `pack_id`;
- versión;
- namespace;
- dependencias;
- política de compatibilidad.

---

# PARTE XIII — MODULARIDAD Y DEGRADACIÓN

## 51. Módulos opcionales

Una implementación futura puede permitir desactivar módulos, pero la degradación debe estar especificada.

Ejemplos:

- sin memoria: relaciones pueden cambiar por eventos directos, pero no hay reactivación episódica;
- sin relaciones: appraisal social usa baseline neutral configurable;
- sin creencias avanzadas: percepción genera una representación subjetiva temporal simple;
- sin ánimo: emociones funcionan sin bias de estado global;
- sin regulación: emociones influyen en decisiones pero no generan coping explícito;
- sin expresión: existe estado interno pero no señal exterior;
- sin aprendizaje social: sólo experiencia propia modifica creencias causales.

No se permiten dependencias implícitas que hagan fallar silenciosamente un preset.

---

# PARTE XIV — DETERMINISMO

## 52. Política determinista

El objetivo es reproducibilidad con mismo estado, eventos, orden y semillas.

La arquitectura debe prever:

- orden estable de procesamiento;
- streams pseudoaleatorios independientes por agente;
- idealmente derivación `seed_global + AgentId`;
- ausencia de dependencia accidental del orden de contenedores;
- replay de estímulos y resultados.

No se promete determinismo bit-a-bit multiplataforma hasta fijar política numérica y de punto flotante.

---

# PARTE XV — EXPLICABILIDAD

## 53. Explicación de decisiones

MSPA debe poder explicar por qué una acción fue preferida.

Ejemplo conceptual:

```text
accion elegida: hablar_con_Aldren
intencion: buscar apoyo
factores:
  tristeza: alta
  confianza en Aldren: alta
  meta de apoyo: activa
  coste temporal: bajo
```

La explicación forma parte del producto, no sólo del debug.

---

## 54. Traza psicológica

El sistema de diagnóstico debe poder reconstruir:

```text
PERCEIVED
BELIEF CREATED / UPDATED
INFERENCE
APPRAISAL
EMOTION
MEMORY ACTIVATED
GOAL CHANGED
ACTION OPTIONS
UTILITY CONSIDERATIONS
CHOSEN ACTION
ACTION RESULT
LEARNING UPDATE
```

---

# PARTE XVI — VALIDACIÓN

## 55. Sandbox antes del freeze

El núcleo no se considera congelado hasta probar al menos:

### Caso Mara

Perfiles diferentes deben producir actualización, rechazo de evidencia, culpa, reconciliación parcial o persistencia del rencor sin scripts especiales por perfil.

### Contradicción

Dos fuentes confiables entregan afirmaciones incompatibles.

### Perdón sin cambio factual

La creencia permanece y la carga emocional cambia por reapreciación.

### Histéresis

Opciones casi empatadas no generan oscilación constante.

### Aprendizaje

Un agente aprende una relación acción-contexto-resultado y ajusta su elección futura.

### Aprendizaje social

Otro agente puede adoptar parcialmente esa expectativa por observación/testimonio.

### Inspiración

Agentes distintos interpretan de forma diferente la misma influencia externa.

### Viajero

Un agente importado conserva recuerdos, conceptos externos y normas aprendidas.

### Divergencia cultural

Dos poblaciones con misma configuración general pero semillas/experiencias distintas deben poder divergir estadísticamente. Al entrar en contacto, deben aparecer fricción, mezcla o aprendizaje sin una regla especial `if culture != culture`.

### Escala

Añadir agentes irrelevantes no cambia la conducta de los relevantes ni rompe presupuesto previsto.

### Determinismo

Mismo estado + mismos estímulos + mismos resultados + mismas seeds produce la misma secuencia lógica bajo una plataforma/política numérica dada.

---

## 56. Métricas poblacionales

Las herramientas externas pueden medir:

- diversidad de conducta;
- distribución de valencia emocional;
- duración de rencores;
- reconciliaciones;
- variedad de metas;
- diversidad de valores y normas;
- polarización;
- convergencia cultural;
- persistencia generacional;
- adopción de prácticas;
- subgrupos;
- estabilidad/deriva a largo plazo.

La cultura no participa como objeto omnisciente en decisiones individuales; estas métricas son observación externa.

---

## 57. Hipótesis, no promesas

MSPA pretende proporcionar mecanismos capaces de producir conducta emergente social y cultural.

No se considera demostrado todavía que genere automáticamente:

- religiones interesantes;
- culturas estables y diversas;
- instituciones complejas;
- innovación sostenida;
- rumores narrativamente útiles.

Esos resultados deben demostrarse mediante sandbox y simulaciones generacionales.

---

## 58. Elementos deliberadamente pospuestos

No forman parte obligatoria del núcleo inicial:

- enfermedades mentales;
- ideologías políticas complejas;
- árbol de rasgos;
- identidad narrativa/autoconcepto avanzado;
- ánimo colectivo global;
- contagio emocional completo;
- reputación pública compleja;
- instituciones políticas formales;
- lenguaje natural;
- generación textual de diálogos.

---

## 59. Estado actual

La arquitectura v2.3 queda como **candidata a prototipo**, no congelada.

Antes de implementación de producción deben cerrarse mediante sandbox:

1. lenguaje y compatibilidad de proposiciones;
2. reglas declarativas de inferencia;
3. fórmula/composición de utility AI;
4. generalización del aprendizaje causal;
5. política de determinismo;
6. portabilidad de estados y packs;
7. pruebas de divergencia cultural.

La especificación de interacción separada se encuentra en `MSPA-INTERACTION-CONTRACT-v0.1.md`.
