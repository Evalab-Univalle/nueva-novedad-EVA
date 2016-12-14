<?php

/**
 * Clase encargada de realizar la retroactividad
 * @author  Maricel Rubio  <marisusa@hotmail.com.co>
 * @access  public
 * @package nomina
 * @copyright   OIT, Universidad del Valle 2004
 * @since   2004-11-24
 * @version 1.0
 *
 */

include_once (PATH_PAQUETE."/nomina/liquidacion/claseLiquidacion.inc");
require_once (PATH_HERRAMIENTAS_REAL."/class.progressbar.inc");

class retroactividadcientoquince {

    /**
    * operación que se realizara
    * @var string
    */
    var $_operacion = "";

    /**
    * Login del usuario que va a manipular la nómina
    * @var string
    */
    var $_usu_login = "";

    /**
    * Aplicación en la que se está trabajando
    * @var string
    */
    var $_aplicacion = "";

    /**
    * Módulo en el cual se está trabajando
    * @var string
    */
    var $_modulo = "";

    /**
    * grupo de trabajo que se conecta a la Aplicación
    * @var string
    */
    var $_grupo    = "";

    /**
    * Objeto de tipo bd
    * @var object
    */
    var $_bd    = "";


    /**
    * Objeto de tipo CONSULTA
    * @var object
    */
    var $_consultaPersonas = "";
    var $_consulta    = "";
    var $_consultaAux = "";
    var $_consultaAux2 = "";
    var $_consultaAux3 = "";

    /**
    * Objeto de tipo HERRAMIENTAS
    * @var object
    */
    var $_herramientas = "";

    var $_objeto_novedad_por_persona;
    var $dias_liquidados = "";
    var $_arreglo_novedad = array();

    /**
    * Nuevo salario, de acuerdo a la novedad de cambio de salario
    * @var float
    */
    var $_nuevo_salario = "";
    
    /**
    * Puntos que tiene al inicio de cada periodo de liquidacion
    * @var float
    */
    var $_puntos_base = array();

    /**
    * Npp_codigo del cambio de salario que genera la retro.
    * @var int
    */
    var $_cds_npp_codigo = "";

    /**
    * Fecha de inicio de la novedad del cambio de salario que genera la retro.
    * @var int
    */
    var $_npp_fecha_novedad = "";

    /* Para determinar si un público es provisional o no
    @var boolean
    */
    var $_es_provisional = "";

     /* Guarda el sueldo nuevo del empleado
    @var int
    */
    var $_sueldoNuevoEnUnMes = "";
    var $_DEPURAR_RETROACTIVIDAD = "";
    
    /* Guarda el codigo de la persona
    @var int
    */
    var $_per_codigo = "";

    /**
     * Constructor de la clase ClaseLiquidacion
     *
     * @author  Carlos A. Mera <carlosmera20@hotmail.com.co>
     * @access    public
     * @copyright OIT, Universidad del Valle 2.003
     * @since     2003-05-31
     * @param     string    $aplicacion     aplicación
     * @param     string    &$bd            objeto bd que se utilizará
     * @param     string    $usu_login      Login del usuario que manejará la nómina
     * @param     string    $grupo          Grupo de trabajo del usuario
     * @see TABLA(), CONSULTA()
     * @return  void    Inicializa la clase
     */
    function retroactividadcientoquince ($aplicacion, $modulo, $grupo, $usu_login, &$bd)
    {
        $this->_aplicacion    = $aplicacion;
        $this->_modulo        = $modulo;
        $this->_bd            = $bd;

        $this->_herramientas  = new HERRAMIENTA();
        $this->_usu_login     = $usu_login;
        $this->_grupo         = $grupo;

        $this->_consultaPersonas      = new CONSULTA($bd);
        $this->_consulta      = new CONSULTA($bd);
        $this->_consultaAux      = new CONSULTA($bd);
        $this->_consultaAux2      = new CONSULTA($bd);
        $this->_consultaAux3      = new CONSULTA($bd);
        $this->_DEPURAR_RETROACTIVIDAD=DEPURAR_RETROACTIVIDAD;

        // Se instancia el objeto novedad por persona
        $this->_objeto_novedad_por_persona = new NovedadPorPersona($this->_aplicacion, $this->_usu_login,
                                                       $this->_grupo,  $this->_bd, $maestro_independiente = TRUE);

        $this->_liquidacion = new ClaseLiquidacion ($aplicacion,$modulo,$grupo,$usu_login,$bd,'t');


        $this->_tabla_liquidacion = new tabla("liquidacionretroactividad", $this->_usu_login, $this->_grupo, $this->_bd,$tabla_independiente=TRUE);
        $this->_tabla_negativos = new tabla("valoresretroactividadceroonegativos", $this->_usu_login, $this->_grupo, $this->_bd,$tabla_independiente=TRUE);
/*para intereses, borra luego cquintero 2004-12-18 */

            $this->tabla_novedad=new TABLA('novedadporpersona',$this->_usu_login, $this->_grupo, $this->_bd);
            $this->tabla_novedad_mvp = new TABLA('mayorvalorpagado',$this->_usu_login, $this->_grupo, $this->_bd);
            $this->tabla_novedad_estado = new TABLA('estadonovedadnodo',$this->_usu_login, $this->_grupo, $this->_bd);
            $this->tabla_novedad_estado->establecerSecuencias(array('enn_codigo'));

/*borrar hasta aqui*/



    }// END function claseLiquidacion

    function hora()
    {
        RETURN date("h:i:s");
    }


    /**
    * Metodo establecerOperacion
    * establece el tipo de operación que se va ha realizar
    * Parámetros:
    * $operacion  Operación que se va ha realizar
    */
    function establecerOperacion ($operacion)
    {
        $this->_operacion = $operacion;
    }// End function establecerOperacion



    /**
     * Este mètodo se encarga de mostrar el formulario para inciar el proceso de liquidaciòn.
     *
     * @author      Carlos A. Mera <carlosmera20@hotmail.com.co>
     * @access      public
     * @copyright   OIT, Universidad del Valle 2.003
     * @since       2003-05-31
     * @return      void
     */
    function mostrarFormulario ()
    {
        include ("fm_SirhLiquidacionRetroactividad.php");
    }// End function mostrarFormulario


    /**
     * Este mètodo inicia la liquidacion de la Retroactividad y hace el llamados a los metodos correspondientes
     *
     *
     * @author      Maricel Rubio
     * @access      public
     * @copyright   OIT, Universidad del Valle 2.004
     * @since       2004-11-05
     * @return
     */

     function evaluarRetroactividadALiquidar ($HTTP_POST_VARS)
     {
     
        if ($HTTP_POST_VARS['pel_codigo'] != "")
        {
			$this->iniciarLiquidacionRetroactividad($HTTP_POST_VARS);
        }
        else
        {
            $prt_codigo  = $HTTP_POST_VARS['prt_codigo'];
            $grl_codigo  = $HTTP_POST_VARS['grl_codigo'];

            $wincomboprt_codigo = $HTTP_POST_VARS['wincomboprt_codigo'];
            $wincombogrl_codigo = $HTTP_POST_VARS['wincombogrl_codigo'];

            $tipo_aplicacion_retroactivo  = $HTTP_POST_VARS['tipo_aplicacion_retroactivo'];

            //include ("fm_SirhLiquidacionRetroactividadPeriodos.php");
        }
    }


    /**
     * Obtiene el npp_codigo de la novedad de cambio de salario que esté dentro de las fechas del periodo
     * retroactivo     *
     *
     * @author      Alexander Varela Galvis
     * @access      public
     * @copyright   OIT, Universidad del Valle 2.008
     * @since       2008-06-13
     * @return      boolean
     */

     function obtenerNppCodigoCambioSalario($pel_codigo,$tnp_codigo){
        $sql = "  SELECT  cds_npp_codigo
                  FROM    novedadporpersona,cambiodesalario
                  WHERE   cds_npp_codigo_origen=npp_codigo
                  AND     npp_tnp_codigo=$tnp_codigo
                  AND     novedadaterminadoflujo(npp_codigo)
                  AND NOT npp_anulada
                  AND     npp_fecha_novedad>='$this->_prt_fecha_inicio'
                  AND     npp_fecha_novedad<='$this->_prt_fecha_final'
                  AND     cds_registro_vigente
                  AND     npp_registro_vigente
                  AND     npp_pel_codigo = $pel_codigo
                  ORDER BY npp_fecha_novedad DESC limit 1";

        $this->_arreglo_novedad[$this->_pel_codigo]=$this->_consulta->obtenerCadenaResultadoConsulta($sql);
        $resp=($this->_arreglo_novedad[$this->_pel_codigo]!="")?TRUE:FALSE;
        return $resp;
    }

     /**
     * Verifica si los excedentes de pago generados por el cambio de salario ya fueron liquidados.
     *
     * @author      Alexander Varela Galvis
     * @access      public
     * @copyright   OIT, Universidad del Valle 2.008
     * @since       2008-06-13
     * @return      boolean
     */

    function validarPelCodigoLiquidado($pel_codigo,$crl_codigo){
        $sql = "  SELECT  true
                  FROM    novedadporpersona,cambiodesalario,
                          personaliquidacion,liquidacionnomina
                  WHERE   novedadaterminadoflujo(npp_codigo)
                  AND     npp_codigo=cds_npp_codigo
                  AND NOT npp_anulada
                  AND     npp_fecha_novedad>='$this->_prt_fecha_inicio'
                  AND     npp_fecha_novedad<='$this->_prt_fecha_final'
                  AND     cds_registro_vigente
                  AND     pel_registro_vigente
                  AND     npp_registro_vigente
                  AND     npp_pel_codigo = pel_codigo
                  AND     pel_codigo =$pel_codigo
                  AND     pel_codigo = lqd_pel_codigo
                  AND     lqd_registro_vigente
                  AND     lqd_npp_codigo = npp_codigo
                  AND     lqd_crl_codigo='$crl_codigo'";

        $resp=($this->_consulta->obtenerCadenaResultadoConsulta($sql)=='t')?TRUE:FALSE;echo "<p>$resp</p>";
        return $resp;
    }


   /**
   * Inicializa las variables de clase liquidación $this->_mes_inicio_periodo_retroactivo y $this->_anno_inicio_periodo_retroactivo
   * se creo con el propósito de usar las variables como parámetro para calcular los excedentes de horas extra, ver pl horas extra publico.
   *
   * @author      Alexander Varela Galvis <alvarela@univalle.edu.co>
   * @access      public
   * @copyright   OITEL, Universidad del Valle 2009
   * @since       2009-08-25
   * @return
   */
    function establecerMesYAnnoInicioPeriodoRetroactivo(){
        $sql="SELECT extract(month from prt_fecha_inicio),extract(year from prt_fecha_inicio)
              FROM   periodoretroactivo
              WHERE  prt_codigo='$this->_prt_codigo'
              AND    prt_registro_vigente";
        $this->_consulta->ejecutarConsulta($sql);
        list($mes_inicio_periodo_retroactivo,$anno_inicio_periodo_retroactivo)=$this->_consulta->sigRegistroArreglo();

        $sql="SELECT mea_codigo
              FROM   mesesanno
              WHERE  mea_numero_mes='$mes_inicio_periodo_retroactivo'
              AND    mea_registro_vigente";
        $this->_liquidacion->_mes_inicio_periodo_retroactivo=$this->_consulta->obtenerCadenaResultadoConsulta($sql);

        $sql="SELECT anno_codigo
              FROM   anno
              WHERE  anno_nombre='$anno_inicio_periodo_retroactivo'
              AND    anno_registro_vigente";
        $this->_liquidacion->_anno_inicio_periodo_retroactivo=$this->_consulta->obtenerCadenaResultadoConsulta($sql);
        
        //Inicializo variables usadas en DescuentosSeguridadSocial
        $this->_liquidacion->_seguridad_social->_mes_inicio_periodo_retroactivo = $this->_liquidacion->_mes_inicio_periodo_retroactivo;
        $this->_liquidacion->_seguridad_social->_anho_inicio_periodo_retroactivo = $this->_liquidacion->_anno_inicio_periodo_retroactivo;
    }


     /**
     * Crea tablas temporales para datos guardados salud y pension
     *
     *
     * @author      Alexander Varela Galvis
     * @access      public
     * @copyright   OIT, Universidad del Valle 2.010
     * @since       2010-04-23
     * @return
     */

    function crearTablaDatosGuardadosSeguridadsocial(){
      
        $sql="SELECT  DISTINCT plq_codigo
              FROM    criterioporgrupoporperiodoliquidacion,PeriodoLiquidacion,criteriogrupoliquidacion,
                      periodicidadcriteriogrupoliquidacion
              WHERE   cgp_plq_codigo=plq_codigo
              AND     cgp_cgl_codigo=cgl_codigo
              AND     plq_pcg_codigo=pcg_codigo
              AND     cgl_grl_codigo='".$this->_grl_codigo."'
              AND     plq_registro_vigente
              AND     cgl_registro_vigente
              AND     cgp_registro_vigente
              AND    plq_fecha_inicio_periodo>='$this->_prt_fecha_inicio'
              AND    plq_fecha_final_periodo<='$this->_prt_fecha_final'";
        $this->_consulta->ejecutarConsulta($sql);
        unset($cadenaPeriodos);
        while(list($plq_codigo)=$this->_consulta->sigRegistroArreglo()){
            $periodos[]=$plq_codigo;
        }
        $cadenaPeriodos=implode(",",$periodos);

        $datosFecha= explode("-",$this->_prt_fecha_inicio);
        $anho       = $datosFecha[0];
        $mesInicio  = $datosFecha[1]*1;
        $datosFecha= explode("-",$this->_prt_fecha_final);
        $mesFin  = $datosFecha[1]*1;

        $sql="CREATE TEMPORARY TABLE datosguardadosseguridadsocialpension_pararetroactividad as (
              SELECT *
              FROM   datosguardadosseguridadsocialpension
              WHERE  dsp_anho=$anho
              AND    dsp_mes>=$mesInicio
              AND    dsp_mes<=$mesFin
              AND    dsp_registro_vigente
              AND    dsp_plq_codigo not IN ($cadenaPeriodos));
              CREATE INDEX datosguardadosseguridadsocialpension_pararetroactividad_idx1 ON datosguardadosseguridadsocialpension_pararetroactividad (dsp_pel_codigo);
              CREATE INDEX datosguardadosseguridadsocialpension_pararetroactividad_idx2 ON datosguardadosseguridadsocialpension_pararetroactividad (dsp_pel_codigo,dsp_anho,dsp_mes);";
              $this->_consulta->ejecutarConsulta($sql);

        $sql="CREATE TEMPORARY TABLE datosguardadosseguridadsocialsalud_pararetroactividad as (
              SELECT *
              FROM   datosguardadosseguridadsocialsalud
              WHERE  dss_anho=$anho
              AND    dss_mes>=$mesInicio
              AND    dss_mes<=$mesFin
              AND    dss_registro_vigente
              AND    dss_plq_codigo not IN ($cadenaPeriodos));
              CREATE INDEX datosguardadosseguridadsocialsalud_pararetroactividad_idx1 ON datosguardadosseguridadsocialsalud_pararetroactividad (dss_pel_codigo);
              CREATE INDEX datosguardadosseguridadsocialsalud_pararetroactividad_idx2 ON datosguardadosseguridadsocialsalud_pararetroactividad (dss_pel_codigo,dss_anho,dss_mes);";
              $this->_consulta->ejecutarConsulta($sql);
              $this->_consulta->liberarMemoria();
    }
    
    
     



    /**
     * Este mètodo inicia la liquidacion de la Retroactividad y hace el llamados a los metodos correspondientes
     *
     *
     * @author      Maricel Rubio
     * @access      public
     * @copyright   OIT, Universidad del Valle 2.004
     * @since       2004-11-05
     * @return
     */

     function iniciarLiquidacionRetroactividad ($HTTP_POST_VARS)
     {
        //$this->_liquidacion->_bd->iniciarTransaccion();
        //echo "<br> INICIO ".$this->hora();
        $this->_liquidacion->_esretroactividad='t';

        $arreglo_datos_retroactividad = array();

        $arreglo_datos_retroactividad = $this->tablaPeriodoRetroactivo($HTTP_POST_VARS);
        $this->_prt_fecha_inicio =   $arreglo_datos_retroactividad[0];
        $this->_prt_fecha_final  =   $arreglo_datos_retroactividad[1];
        $this->_plq_codigo_pago  =   $arreglo_datos_retroactividad[2];
        $this->_prt_valor        =   $arreglo_datos_retroactividad[3];
        $this->_prt_porcentaje   =   $arreglo_datos_retroactividad[4];
        $this->_prt_tipo         =   $arreglo_datos_retroactividad[5];
        $this->_grl_codigo       =   ($HTTP_POST_VARS['tipo_aplicacion_retroactivo']=="PERSONA")?$HTTP_POST_VARS['pel_grl_codigo']:$HTTP_POST_VARS['grl_codigo'];
        $this->_prt_codigo       =   $HTTP_POST_VARS['prt_codigo'];
        $this->_nombregrupo = $this->atributoTabla("grl_nombre","grupoliquidacion","grl_codigo=$this->_grl_codigo","grl_registro_vigente");

        $sql="SELECT crl_codigo,crl_nombre
              FROM   criterioliquidacion
              WHERE  crl_registro_vigente";
        $this->_consulta->ejecutarConsulta($sql);
        while(list($crl_codigo,$crl_nombre)=$this->_consulta->sigRegistroArreglo()){
          $this->_nombreCriterios[$crl_codigo]=$crl_nombre;
        }

        //echo "<br> establecerMesYAnnoInicioPeriodoRetroactivo ".$this->hora();
        $this->establecerMesYAnnoInicioPeriodoRetroactivo();
        //echo "<br> crearTablaDatosGuardadosSeguridadsocial ".$this->hora();
        $this->crearTablaDatosGuardadosSeguridadsocial();
        //echo "<br> obtenerPeridosYaLiquidados ".$this->hora();
        $this->arreglo_plq_retroactivo = $this->obtenerPeridosYaLiquidados();
        //echo "<br><pre> arreglo_plq_retroactivo";print_r($this->arreglo_plq_retroactivo);
        //echo "<br> obtenerCodigosPeriodoLiquidacion ".$this->hora();
        $this->_arreglo_plq_cod_global = $this->obtenerCodigosPeriodoLiquidacion("GLOBAL");
        //echo "<br><pre> this->_arreglo_plq_cod_global";print_r($this->_arreglo_plq_cod_global);
        foreach($this->_arreglo_plq_cod_global as $plq_codigo => $tipo){
              $this->_criteriosGrupo[$plq_codigo]['criterios']=$this->obtenerCriteriosLiquidacionGrupo($plq_codigo);
              $this->_criteriosGrupo[$plq_codigo]['periodicidad']=$tipo;
        }

        $this->_liquidacion->_grl_codigo=$this->_grl_codigo;
        $this->_liquidacion->_plq_codigo=$this->arreglo_plq_retroactivo[0];
        $this->_liquidacion->_fechaInicioLiquidacion = $this->_prt_fecha_inicio ;
        $this->_liquidacion->_fechaFinalizacionLiquidacion = $this->_prt_fecha_final ;
        $this->_liquidacion->criteriosBaseSeguridadSocial("SALUD");
        $this->_liquidacion->criteriosBaseSeguridadSocial("PENSION");
        $this->_liquidacion->inicializarConstantesConVigenciaParaSeguridadSocial();
        $this->_liquidacion->inicializarArregloGruposConNovedadDeVacaciones();
        $this->_liquidacion->_prt_codigo=$this->_prt_codigo;
        


        $this->_nombreConstanteDescuentoPension = $this->_liquidacion->_seguridad_social->obtenerNombreConstanteDescuento($this->_grl_codigo, 'PENSION') ;
        $this->_nombreConstanteDescuentoSalud = $this->_liquidacion->_seguridad_social->obtenerNombreConstanteDescuento($this->_grl_codigo, 'PENSION') ;
        $this->_porcentaje_descuento_pension = $this->_liquidacion->_seguridad_social->obtenerValorConstanteConVigencia($this->_nombreConstanteDescuentoPension, $this->_prt_fecha_inicio);
        $this->_porcentaje_descuento_salud = $this->_liquidacion->_seguridad_social->obtenerValorConstanteConVigencia($this->_nombreConstanteDescuentoSalud, $this->_prt_fecha_inicio);
        //echo "<br> foreach arreglo_plq_cod_global ".$this->hora();




        if($HTTP_POST_VARS['grl_codigo']=="")
        {
            $this->_grl_codigo    =   $HTTP_POST_VARS['pel_grl_codigo'];
        }

       // POR PERSONA
        if ($HTTP_POST_VARS['pel_codigo'] !="")
        {
            $this->_bd->iniciarTransaccion();
            $this->_tipo="PERSONA";
            unset($this->_nuevo_salario);

            $this->_pel_codigo = $HTTP_POST_VARS['pel_codigo'];

            $this->borrarTablaLiquidacionRetroactivo("PERSONA");

            $this->establecerParametrosLiquidacion($HTTP_POST_VARS,$this->_pel_codigo);
            $this->_liquidacion->eliminarEmpleadosRetirados();

            $ArregloAImprimir = array();
            $ArregloAImprimirValoresCero = array();
            $ArregloAImprimirValoresNegativos = array();
            $liquidar=TRUE;
            if($this->_prt_tipo=='NIVELACION'){
                $tnp_codigo=12;
                $crl_codigo=104;
                /*if($this->_prt_tipo=='CAMBIOSALARIO'){
                  $tnp_codigo=60;
                  $crl_codigo=111;
                }*/

                /*if(!$this->obtenerNppCodigoCambioSalario($this->_pel_codigo,$tnp_codigo)){
                  $error = new error("ERROR","ERROR1026");
                  $mesg = $error->retornarMensajeError();
                  print "<center>$mesg</center><br><br>";
                  $liquidar=FALSE;
                }else{*/
                    if($this->validarPelCodigoLiquidado($this->_pel_codigo,$crl_codigo)){
                      $cadena= "(pel_codigo=$this->_pel_codigo)";
                      $error = new error("ERROR","ERROR1025");
                      $mesg = $error->retornarMensajeError();
                      eval( "\$mesg = \"$mesg\";" );
                      print "<center>$mesg</center><br><br>";
                      $liquidar=FALSE;
                    }
                //}
            }

            if($liquidar)
				$resp = $this->liquidacionRetroactivaPersona($ArregloAImprimir, $ArregloAImprimirValoresNegativos, $ArregloAImprimirValoresCero);

            if ($resp)
            {
                //echo "<br> FIN CLICLO ".$this->hora();
                $this->_bd->confirmarTransaccion();
                $this->_liquidacion->_bd->confirmarTransaccion();
                $error = new error("ERROR","ERROR879");
                $mesg = $error->retornarMensajeError();
                print "<center>$mesg</center><br><br>";
                $this->imprimirResultados("Retroactivos Generados:");
            }
            else
            {
                $this->_bd->abortarTransaccion();
                $this->_liquidacion->_bd->abortarTransaccion();
                $error = new error("ERROR","ERROR881");
                $mesg = $error->retornarMensajeError();
                print "<center>$mesg</center>";
            }
        }
        // POR  GRUPO
        else
        {
            $mesDeLaRetroactividad = $HTTP_POST_VARS['radio'];
            if (strlen($mesDeLaRetroactividad) == 1)
            {
                $this->mesDeLaRetroactividad = "0".$mesDeLaRetroactividad;
            }
            else
            {
                $this->mesDeLaRetroactividad = $mesDeLaRetroactividad;
            }

            $this->_tipo = "GRUPO";

            $HTTP_POST_VARS['tipo_liquidacion'] = "liquidacionprenominatemporal";

            $HTTP_POST_VARS['tipo_periodicidad'] = "PERIODICO";

            //alvarela:2008-04-02: Adiciono la siguiente inicializacion de variables:
            //=========
            $this->_manejar_transaccion = FALSE;
            $this->_liquidacion->_imprimirResultados    = FALSE;
            $this->_liquidacion->_imprimirResultados    = FALSE;
            $this->_liquidacion->_manejar_transaccion   = FALSE;
            $this->_liquidacion->_verificar_aprobacion  = TRUE;
            $this->_liquidacion->_comfirmar_liquidacion = FALSE;
            $this->_liquidacion->_verificar_procesos_ejecutados = FALSE;
            $this->liquidacionRetroactividad = TRUE;
            //==========

            $this->_liquidacion->establecerParametrosLiquidacion($HTTP_POST_VARS);

            //$this->arreglo_plq_retroactivo = $this->obtenerPeridosYaLiquidados();
            $this->_bd->iniciarTransaccion();
            $this->borrarTablaLiquidacionRetroactivo("GRUPO");


            if (count($this->arreglo_plq_retroactivo) > 0)
            {
                $this->_liquidacion->_periodos_a_liquidar = $this->arreglo_plq_retroactivo;
            }
            else
            {
                echo "<br><br><center>NO HAY PERIODOS A LIQUIDAR ENTRE LAS FECHAS DEL RETROACTIVO</center>";
                exit;
            }

            $this->_liquidacion->generarTablasTemporales();
            $this->_liquidacion->eliminarEmpleadosRetirados();

            $this->_liquidacion->_periodos_a_liquidar = array();

            //echo "<br> INI personasCandidatas ".$this->hora();
            $this->personasCandidatas($this->_grl_codigo);
            //echo "<br> FIN personasCandidatas ".$this->hora();
            //echo "<pre>PEL_CODIGOS A LIQUIDAR <BR>";print_r($arreglo_pel_cod);

            $ArregloAImprimir = array();
            $ArregloAImprimirValoresCero = array();
            $ArregloAImprimirValoresNegativos = array();

            //alavarea:2007-03-26: Coloco las líneas correspondientes al despligue de la barra.
            $cantidad=$this->_consultaPersonas->numFilas();
            if ($cantidad > 0)
            {
                if($this->_prt_tipo=='NIVELACION'){
                    $tnp_codigo=12;
                    $crl_codigo=104;
                }
                /*if($this->_prt_tipo=='CAMBIOSALARIO'){
                      $tnp_codigo=60;
                      $crl_codigo=111;
                }*/


                $prb=$this->CrearBarra($cantidad);
                $conteo_barra = 1 ;
                //echo "<br> INI CLICLO ".$this->hora();
                 while(list($pel_codigo,$per_codigo)=$this->_consultaPersonas->sigRegistroArreglo()){
                    flush();
                    ob_flush();
                    ob_clean();
                    //ECHO "<br>conteo_barra $conteo_barra pel_codigo $pel_codigo";
                    $this->_pel_codigo = $pel_codigo;
                    /*if($this->_prt_tipo=='NIVELACION' || $this->_prt_tipo=='CAMBIOSALARIO'){
                        if(!$this->obtenerNppCodigoCambioSalario($this->_pel_codigo,$tnp_codigo)){
                          $error = new error("ERROR","ERROR1026");
                          $mesg = $error->retornarMensajeError();
                          print "<center>$mesg</center><br><br>";
                          return FALSE;
                        }else{
                            if($this->validarPelCodigoLiquidado($this->_pel_codigo,$crl_codigo)){
                              $cadena= "(pel_codigo=$pel_codigo)";
                              $error = new error("ERROR","ERROR1025");
                              $mesg = $error->retornarMensajeError();
                              eval( "\$mesg = \"$mesg\";" );
                              print "<center>$mesg</center><br><br>";
                              return FALSE;
                            }
                        }
                     }*/

                    $prb->moveStep($conteo_barra);
                    if ($pel_codigo <> "")
                    {
                        unset($this->_nuevo_salario);
                        $this->_liquidacion->_per_codigo=$per_codigo;
                        $resp = $this->liquidacionRetroactivaPersona($ArregloAImprimir, $ArregloAImprimirValoresNegativos, $ArregloAImprimirValoresCero);
                    }

                    $conteo_barra++;
                    if(!$resp)
                    {
                        break;
                    }else{
                          $this->_consulta->liberarMemoria();
                          $this->_consultaAux->liberarMemoria();
                          $this->_consultaAux2->liberarMemoria();
                          $this->_consultaAux3->liberarMemoria();
                    }
                    //echo "<br>POAKA $this->_pel_codigo";//.memory_get_usage(true) . "\n";
                }// while(list($pel_codigo,$per_codigo)=$this->_consultaPersonas->sigRegistroArreglo()){
                //echo "<br> FIN CLICLO ".$this->hora();

                if ($resp)
                {
                      //echo "<br> TERMINO ".$this->hora();
                    $this->_consultaPersonas->liberarMemoria();
                    //$this->_liquidacion->_bd->confirmarTransaccion();
                    $this->_bd->confirmarTransaccion();
                    $error = new error("ERROR","ERROR879");
                    $mesg = $error->retornarMensajeError();
                    print "<center>$mesg</center><br><br>";
                    $this->imprimirResultados("Retroactivos Generados:");
                    //$this->imprimirResultados($ArregloAImprimirValoresNegativos, "Valores NEGATIVOS Generados en la Retroactividad");
                    //$this->imprimirResultados($ArregloAImprimirValoresCero, "Valores CERO en la liquidación");
                }
                else
                {
                    $this->_bd->abortarTransaccion();
                    //$this->_liquidacion->_bd->abortarTransaccion();
                    $error = new error("ERROR","ERROR881");
                    $mesg = $error->retornarMensajeError();
                    print "<center>$mesg</center>";
                }
            }
            else
            {
                    //$this->_bd->abortarTransaccion();
                    //$this->_liquidacion->_bd->abortarTransaccion();
                    $error = new error("ERROR","ERROR935");
                    $mesg = $error->retornarMensajeError();
                    print "<center>$mesg</center>";
            }
        }
    }



    /**
     * Este mètodo borra las tablas temporales que se han creado en Liquidacion
     *
     * @author      Carlos A. Mera <carlosmera20@hotmail.com.co>
     * @access      public
     * @copyright   OIT, Universidad del Valle 2.004
     * @since       2004-11-11
     * @return      string
     */
    function borrarTablasTemporalesLiquidacion ()
    {
        $consultaLiquidacion = new Consulta($this->_bd);

        // Se borran las tablas temporales de liquidacion
        $SQL = "DROP TABLE esp_nov;
                DROP TABLE novedadesporpersonatemporal;
                DROP TABLE novedades_periodicasesp_nov;
                DROP TABLE novedades_periodicas;
                DROP TABLE novedadesporpersonatemporalafectantiempolaboradoesp_nov;
                DROP TABLE novedadesporpersonatemporalafectantiempolaborado;
                DROP TABLE novedadesderetencionenlafuenteesp_nov;
                DROP TABLE novedadesderetencionenlafuente;
                DROP TABLE nov_vinvinculacioncontratistaestatalliq;
                DROP TABLE nov_vinvinculacioncontratistadocentehoracatedraliq;
                DROP TABLE nov_vin_espvinculacioncontratistadocenteliq;
                DROP TABLE nov_vinvinculacioncontratistadocenteliq;
                DROP TABLE nov_vin_espvinculacioncontratistaliq;
                DROP TABLE nov_vinvinculacioncontratistaliq;
                DROP TABLE nov_vin_esp_liq;
                DROP TABLE novedadesvinculacionlaboralliq;
                DROP TABLE novedadesvinculaciontemporalliq;
                DROP TABLE adicionesesp_nov;
                DROP TABLE adiciones;
                ";
        $consultaLiquidacion->ejecutarConsulta($SQL);
    }


     /**
    * Obtiene los plq_codigo que no fueron tenidos en cuenta pues el empleado se encontraba en vacaciones
    *
    * @author      Alexander Varela Galvis <alvarela@univalle.edu.co>
    * @access      public
    * @copyright   OITEL, Universidad del Valle 2.008
    * @since       2008-12-09
    * @return      array.
    */

    function incluirPeriodosDeLiquidacionNoPagadosPorVacaciones($arreglo_plq_cod){
    
        if (count($this->_arreglo_plq_cod_global)){
            $plq_codigos_liquidados = implode(",",array_keys($this->_arreglo_plq_cod_global));
        }
        $sql="SELECT plq_codigo
              FROM   periodoliquidacion,novedadporpersona,vacaciones
              WHERE  npp_pel_codigo=$this->_pel_codigo
              AND    npp_codigo=vac_npp_codigo
              AND    npp_fecha_novedad<=plq_fecha_inicio_periodo
              AND    vac_fecha_finalizacion_novedad>=plq_fecha_final_periodo
              AND NOT npp_anulada
              AND   novedadaterminadoflujo(npp_codigo)
              AND   plq_fecha_inicio_periodo>='$this->_prt_fecha_inicio'
              AND   extract (year from npp_fecha_novedad)=extract(year from date ('$this->_prt_fecha_final'))
              AND   plq_codigo in ($plq_codigos_liquidados)
              ";
         $this->_consultaAux3->ejecutarConsulta($sql);
         while(list($plq_codigo)=$this->_consultaAux3->sigRegistroArreglo()){
              $arreglo_plq_cod[]=$plq_codigo;
         }
         return $arreglo_plq_cod;
    }
    
     /**
    * Liquida las vacaciones que se solapan con el periodo retroactivo, pero que no
    * fueron liquidadas en un plq que se incluye dentro del periodo retroactivo.
    * @author      Alexander Varela Galvis <alvarela@univalle.edu.co>
    * @access      public
    * @copyright   OITEL, Universidad del Valle 2.008
    * @since       2008-12-10
    * @return      void.
    */
    function obtenerPlqCodigoDeVacacionesQueInicianEnPeriodoRetroactivoPeroForaneasAEste(&$ArregloAImprimir, &$ArregloAImprimirValoresNegativos, &$ArregloAImprimirValoresCero, &$arreglo_plq_cod){
        unset($this->_vac_plq_codigo_foraneo);
        unset($complemento);
        if (count($arreglo_plq_cod)){
            $plq_codigos_liquidados = implode(",",$arreglo_plq_cod);
            $complemento="AND    vac_plq_codigo not in ($plq_codigos_liquidados)";
        }

        $sql=" SELECT vac_plq_codigo
               FROM   liquidacionnomina,vacaciones,
                      novedadporpersona,periodoliquidacion
               WHERE  lqd_pel_codigo=$this->_pel_codigo
               AND    lqd_crl_codigo=14
               AND    lqd_npp_codigo=npp_codigo
               AND    lqd_pel_codigo=npp_pel_codigo
               AND    npp_codigo=vac_npp_codigo
               AND not npp_anulada
               $complemento
               AND    novedadaterminadoflujo(npp_codigo)
               AND    npp_fecha_novedad BETWEEN '$this->_prt_fecha_inicio' AND '$this->_prt_fecha_final'
               AND    vac_plq_codigo=plq_codigo
               AND    npp_registro_vigente
               AND    plq_registro_vigente
               AND    lqd_registro_vigente
               AND    vac_registro_vigente";
               
       $this->_consultaAux3->ejecutarConsulta($sql);
       if($this->_consultaAux3->numFilas()>0){
          list($this->_vac_plq_codigo_foraneo)=$this->_consultaAux3->sigRegistroArreglo();
          array_unshift($arreglo_plq_cod,$this->_vac_plq_codigo_foraneo);
          
          $DELETE="dsp_pel_codigo=$this->_pel_codigo
                AND    dsp_plq_codigo=$this->_vac_plq_codigo_foraneo
                AND    dsp_registro_vigente";
          $tabla_datospension = new tabla("datosguardadosseguridadsocialpension_pararetroactividad", $this->_usu_login, $this->_grupo, $this->_bd,$tabla_independiente=TRUE);
          $tabla_datospension->deleteReal($DELETE,$rest = "", $interfaz = TRUE, $cadena_delete = "", $delete_a_la_fija=TRUE);
          
          $DELETE="dss_pel_codigo=$this->_pel_codigo
                AND    dss_plq_codigo=$this->_vac_plq_codigo_foraneo
                AND    dss_registro_vigente";
          $tabla_datossalud = new tabla("datosguardadosseguridadsocialsalud_pararetroactividad", $this->_usu_login, $this->_grupo, $this->_bd,$tabla_independiente=TRUE);               
          $tabla_datossalud->deleteReal($DELETE,$rest = "", $interfaz = TRUE, $cadena_delete = "", $delete_a_la_fija=TRUE);
          unset($tabla_datospension,$tabla_datossalud);
       }
     }  

      
  /**
    * Elimina los valores que se generaron al liquidar un periodo foranea al 
    * periodo retroactivos, se incluye un periodo foraneo cuando en este se 
    * liquidó el valor de vacaciones que pertenecían al periodo retroactivo.        
    * @author      Alexander Varela Galvis <alvarela@univalle.edu.co>
    * @access      public
    * @copyright   OITEL, Universidad del Valle 2.012
    * @since       2012-05-16
    * @return      void.
    */
     function eliminarValoresDeCriteriosAjenosAVacacionesDePeriodosForaneosAlPeriodoRetroactivo(){
           if(isset($this->_vac_plq_codigo_foraneo)){
               $DELETE="lra_pel_codigo=$this->_pel_codigo
                       AND    lra_prt_codigo='$this->_prt_codigo'
                       AND    lra_registro_vigente
                       AND    lra_crl_codigo_liquidado not in (14,79,33,15)
                       AND    lra_plq_codigo_liquidado='$this->_vac_plq_codigo_foraneo'";
               $this->_tabla_liquidacion->deleteReal($DELETE);
           }    
     }
    
    /**
    * Inserta el registro del criterio al cual se le calculó cero o negativo
    * como valor de retroactividad.    
    * @author      Alexander Varela Galvis <alvarela@univalle.edu.co>
    * @access      public
    * @copyright   OITEL, Universidad del Valle 2.010
    * @since       2010-11-22
    * @param       $datos array - arreglo de datos del registro    
    * @return      void.
    */

    function insertarValorNegativoOCero($datos){
        $datos['lrn_pel_codigo']=$this->_pel_codigo;
        $datos['lrn_prt_codigo']=$this->_prt_codigo;
        $datos['lrn_plq_codigo']=$this->_plq_codigo;                                        
        $this->_tabla_negativos->establecerColumnas($datos);
        $this->_tabla_negativos->establecerSecuencias(array("lrn_codigo"));
        $this->_tabla_negativos->insert($rest = "", $interfaz = TRUE, $cadena_insert = "", $insertar_a_la_fija=TRUE);    
    }


    /**
    * Inserta el valor de la retroactividad a pagar, este método se usa sólo para insertar los valores
    * de vacaciones, primavacaciones y bonificación de recreación, de aquellas vacaciones que iniciaron en el
    * periodo retroactivo pero que fueron pagadas fuera de él.
    * @author      Alexander Varela Galvis <alvarela@univalle.edu.co>
    * @access      public
    * @copyright   OITEL, Universidad del Valle 2.009
    * @since       2009-05-20
    * @return      void.
    */

    function insertarCriterio(&$ArregloAImprimir,$valorRetro,$plq_codigo,$npp_codigo,$dias_liquidados,$crl_codigo,$nombre_criterio){
          $DATOS_RETROACTIVOS["lra_pel_codigo"]            = $this->_pel_codigo;
          $DATOS_RETROACTIVOS["lra_prt_codigo"]            = $this->_prt_codigo;
          $DATOS_RETROACTIVOS["lra_valor_liquidado"]       = round($valorRetro);
          $DATOS_RETROACTIVOS["lra_crl_codigo_liquidado"]  = $crl_codigo;
          $DATOS_RETROACTIVOS["lra_plq_codigo_liquidado"]  = $plq_codigo;
          $DATOS_RETROACTIVOS["lra_npp_codigo_liquidado"]  = $npp_codigo;
          $DATOS_RETROACTIVOS["lra_dias_liquidados"]       = $dias_liquidados;
          $resp = TRUE;

          //Inserta directamente en la tabla liquidacionretroactividad
          if(!$this->insertar($DATOS_RETROACTIVOS))
          {
              $this->_bd->abortarTransaccion();
              $error = new error("AVISO","AVISO4");
              $mesg = $error->retornarMensajeError();
              print "<center>$mesg</center>";
              $resp = FALSE;
          }else{
              $ArregloAImprimir[$this->_pel_codigo]["Criterios"][$nombre_criterio] += $DATOS_RETROACTIVOS['lra_valor_liquidado'];
              //se inserta el valor en la table liquidaciontempo para que este valor sea tenido en cuenta en los descuentos.
                $DATOS_LIQUIDACION["lqp_pel_codigo"]      = $this->_pel_codigo;
                $DATOS_LIQUIDACION["lqp_plq_codigo"]      = $plq_codigo;
                $DATOS_LIQUIDACION["lqp_grl_codigo"]      = $this->_grl_codigo;
                $DATOS_LIQUIDACION["lqp_crl_codigo"]      = $crl_codigo;
                $DATOS_LIQUIDACION["lqp_valor_criterio"]  = round($valorRetro);
                $DATOS_LIQUIDACION["lqp_dias_liquidados"] = $dias_liquidados;
                //$DATOS_LIQUIDACION["lqp_npp_codigo"]      = $variables_liquidacion["npp_codigo"]["valor"];
               if(!$this->_liquidacion->almacenarNomina($DATOS_LIQUIDACION)){
                  print "Ocurrió un error al almacenar los datos para vacaciones del pel_codigo $this->_pel_codigo y plq_codigo $plq_codigo";
                  $resp = FALSE;
               }
          }

          return $resp;
    }


    /**
    * Calcula la retro de criterios específicos, se creó para calcular la retro de
    * los criterios prima de vacaciones y bonificación de recreación de los casos
    * donde el periodo de disfrute de las vacaciones inicia dentro del periodo retroactivo
    * pero estas fueron pagadas por fuera de este úlitmo.
    * @author      Alexander Varela Galvis <alvarela@univalle.edu.co>
    * @access      public
    * @copyright   OITEL, Universidad del Valle 2.009
    * @since       2009-05-20
    * @return      integer.
    */
    function calcularRetroDeCriteriosGeneradosPorVacaciones(&$ArregloAImprimir, $plq_codigo, $npp_codigo, $plq_fecha_inicio_periodo, $plq_fecha_final_periodo, $dias_liquidados, $crl_codigo, $nombre_criterio){
        $SQL = "SELECT lqd_valor_criterio
                FROM   liquidacionnomina
                WHERE  lqd_plq_codigo=$plq_codigo
                AND    lqd_pel_codigo=$this->_pel_codigo
                AND    lqd_crl_codigo=$crl_codigo
                AND    lqd_registro_vigente";
        $valorLiquidadoAntes=$this->_consultaAux3->obtenerCadenaResultadoConsulta($SQL);
        
         $SQL = "SELECT  sum(lra_valor_liquidado)
                FROM    liquidacionretroactividad
                WHERE   lra_pel_codigo = '$this->_pel_codigo'
                AND     lra_crl_codigo_liquidado = '$crl_codigo'
                AND     lra_registro_vigente
                AND     lra_plq_codigo_liquidado=(SELECT b.lqd_plq_codigo
                                        				  FROM   liquidacionnomina AS a, liquidacionnomina AS b
                                        				  WHERE  a.lqd_pel_codigo='$this->_pel_codigo'
                                        				  AND    a.lqd_npp_codigo=$npp_codigo
                                        				  AND    a.lqd_crl_codigo=14
                                        				  AND    a.lqd_plq_codigo=b.lqd_plq_codigo
                                        				  AND    b.lqd_crl_codigo='$crl_codigo'
                                        				  AND    b.lqd_pel_codigo='$this->_pel_codigo')
            		AND     lra_prt_codigo in     ( SELECT distinct lra_prt_codigo
                                    						FROM   liquidacionretroactividad,periodoretroactivo,liquidacionnomina
                                    						WHERE  lra_pel_codigo='$this->_pel_codigo'
                                    						AND    lra_prt_codigo=prt_codigo
                                    						AND    prt_plq_codigo=lqd_plq_codigo
                                    						AND    lqd_pel_codigo='$this->_pel_codigo')";
        $ValoresRetros=$this->_consultaAux3->obtenerCadenaResultadoConsulta($SQL);

        switch($this->_grl_codigo){
          case '1':
              switch($crl_codigo){
                  case '33':
                    $SQL = "select primavacacionesdocentenombradoresolucion115($this->_pel_codigo, '$plq_fecha_inicio_periodo', '$plq_fecha_final_periodo','$plq_codigo','$npp_codigo', 't')";
                  break;
                  case '15':
                    $SQL = "select bonificacionrecreaciondocentenombradoresolucion115($this->_pel_codigo, $plq_codigo, '$plq_fecha_inicio_periodo' , '$plq_fecha_final_periodo','t')";
                  break;
              }
          break;
          case '2':
              switch($crl_codigo){
                  case '33':
                    $SQL = "select primavacacionesdocentenombradodecreto1279($this->_pel_codigo, '$plq_fecha_inicio_periodo', '$plq_fecha_final_periodo','$plq_codigo','t')";
                  break;
                  case '15':
                    $SQL = "select bonificacionrecreaciondocentenombradodecreto1279($this->_pel_codigo, $plq_codigo, '$plq_fecha_inicio_periodo' , '$plq_fecha_final_periodo','t')";
                  break;
              }
          break;
          case '3':
              switch($crl_codigo){
                  case '33':
                    $SQL = "select primavacacionespublico($this->_pel_codigo, $plq_codigo, '$plq_fecha_inicio_periodo', '$plq_fecha_final_periodo', 't')";
                  break;
                  case '15':
                    $SQL = "select bonificacionrecreacionpublico($this->_pel_codigo, $plq_codigo, '$plq_fecha_inicio_periodo' , '$plq_fecha_final_periodo','t')";
                  break;
              }
          break;

          case '5':
              switch($crl_codigo){
                  case '33':
                    $SQL = "select primavacacionesoficialconvencioncolec($this->_pel_codigo, $plq_codigo)";
                  break;
                  case '15':
                    $SQL = "select bonificacionrecreacionoficialconvencioncolec($this->_pel_codigo, $this->_sueldoNuevoEnUnMes, $plq_codigo, 30)";
                  break;
              }
          break;

          case '6':
              switch($crl_codigo){
                  case '33':
                    $SQL = "select primavacacionesoficialconprestdeley($this->_pel_codigo, $plq_codigo, '$plq_fecha_inicio_periodo')";
                  break;
                  case '15':
                    $SQL = "select bonificacionrecreacionoficialconprestdeley($this->_pel_codigo, $this->_sueldoNuevoEnUnMes, $plq_codigo, 30)";
                  break;
              }
          break;
        }

        $nuevoValor=$this->_consultaAux3->obtenerCadenaResultadoConsulta($SQL);
        $RetroAPagar=$nuevoValor-$valorLiquidadoAntes-$ValoresRetros;
        $this->insertarCriterio($ArregloAImprimir,$RetroAPagar,$plq_codigo,$npp_codigo,$dias_liquidados,$crl_codigo,$nombre_criterio);
    }


    /**
    * Verifica si el pel_codigo de un publico es provisional.
    *
    * @author      Alexander Varela Galvis
    * @access      public
    * @copyright   OITEL, Universidad del Valle 2.010
    * @since       2010-05-18
    */

    /*function es_provisional ()
    {
          unset($this->_es_provisional);
          $sql="
              SELECT  vpb_es_provisional
              FROM    vinculacionpublico,novedadporpersona
              WHERE   vpb_npp_codigo=npp_codigo
                AND   npp_pel_codigo=$this->_pel_codigo
                AND   vpb_registro_vigente
                AND   npp_registro_vigente
                AND   NOT npp_anulada
                AND   novedadaterminadoflujo(npp_codigo)
          ";
          if($this->_consulta->obtenerCadenaResultadoConsulta($sql)=='t'){
            $this->_es_provisional = TRUE;
          }ELSE{
            $this->_es_provisional = FALSE;
          }
    }*/



   /**
    * Realiza la liquidacion Retroactiva a una persona
    *
    * @author      Maricel rubio
    * @access      public
    * @copyright   OIT, Universidad del Valle 2.004
    * @since       2004-11-30
    */

    function liquidacionRetroactivaPersona (&$ArregloAImprimir, &$ArregloAImprimirValoresNegativos, &$ArregloAImprimirValoresCero)
    {
        $arreglo_plq_cod = $this->obtenerCodigosPeriodoLiquidacion();
        $arreglo_plq_cod = $this->incluirPeriodosDeLiquidacionNoPagadosPorVacaciones($arreglo_plq_cod);
        $arreglo_plq_cod = array_unique($arreglo_plq_cod);
        $arreglo_novedades_acumuladas; //Voy agregando novedades a medida que voy pasando por los periodos de liquidacion
        $this->_cadena_arreglo_plq_cod = implode($arreglo_plq_cod,",");
        $this->_liquidacion->_seguridad_social->_plqs_del_periodo_retroactivo=$this->_cadena_arreglo_plq_cod;
        $this->obtenerPlqCodigoDeVacacionesQueInicianEnPeriodoRetroactivoPeroForaneasAEste($ArregloAImprimir, $ArregloAImprimirValoresNegativos, $ArregloAImprimirValoresCero, $arreglo_plq_cod);
        $this->_puntos_base = $this->encontrarPuntosIniciales(); //Arreglo con los puntos iniciales
        $resp = FALSE;

		//Si hay varios periodos de liquidación
        if (count($arreglo_plq_cod) > 0) {

            if ($this->_tipo=="PERSONA") {
              $SQL = "SELECT  per_codigo,per_doc_ide_numero, per_primer_nombre,
                              per_segundo_nombre, per_primer_apellido, per_segundo_apellido
                      FROM    persona, personaliquidacion
                      WHERE   pel_per_codigo = per_codigo
                              AND per_registro_vigente
                              ANd pel_codigo = '$this->_pel_codigo'";

              $this->_consulta->ejecutarConsulta($SQL);
              list($this->_liquidacion->_per_codigo, $per_doc_ide_numero, $per_primer_nombre, $per_segundo_nombre, $per_primer_apellido, $per_segundo_apellido)=$this->_consulta->sigRegistroArreglo();

              $per_nombre = $per_doc_ide_numero."-".$per_primer_nombre." ".$per_segundo_nombre." ".$per_primer_apellido." ".$per_segundo_apellido;
			  $this->_per_codigo = $this->_liquidacion->_per_codigo;

              $ArregloAImprimir[$this->_pel_codigo]["Nombre"] = $per_nombre;
              $ArregloAImprimir[$this->_pel_codigo]["Criterios"] = array();
            } else {
				$SQL = "SELECT pel_per_codigo from personaliquidacion
						WHERE pel_codigo = $this->_pel_codigo
						AND   pel_registro_vigente";
				$this->_per_codigo = $this->_consulta->obtenerCadenaResultadoConsulta($SQL);
			}

            //echo "<br><br>----->ANTES FOREACH ".$this->hora();
            //$arreglo_plq_cod = array(837);
            $fecha_final_plq = "";
            $existe_retroactivo = FALSE; //Variable para comprobar que no se hagan calculos si no se ha llegado a la primera novedad del periodo retroactivo
            foreach ($arreglo_plq_cod as $campo => $plq_codigo)
            {
                $this->_plq_codigo = $plq_codigo;

                //Consultamos las novedades del periodo de liquidacion
                $arreglo_plq_novedades = array();
                $SQL = "SELECT	 ndc_titulos_obtenidos, ndc_labor_docente, ndc_experiencia_profesional, ndc_experiencia_academica, ndc_produccion_intelectual, ndc_idiomas, ndc_distincion_academica, npp_fecha_novedad, ndc_npp_codigo
						FROM 	 novedadporpersona, nivelaciondocentecientoquince, periodoliquidacion
						WHERE	 plq_codigo = $this->_plq_codigo
						AND 	 npp_pel_codigo = $this->_pel_codigo
						AND		 npp_codigo = ndc_npp_codigo
						AND		 npp_fecha_novedad BETWEEN plq_fecha_inicio_periodo AND plq_fecha_final_periodo
						AND 	 novedadaterminadoflujo(npp_codigo)
						AND		 npp_registro_vigente
						AND		 ndc_registro_vigente
						AND		 plq_registro_vigente
						ORDER BY npp_fecha_novedad DESC";

				$this->_consultaAux->ejecutarConsulta($SQL);
				$contador_novedades = 0;
				//--Arreglo que va acumulando los puntos
				$sumatoria_puntos = array();
				$sumatoria_puntos[6] = 0;
				$sumatoria_puntos[7] = 0;
				$sumatoria_puntos[8] = 0;
				$sumatoria_puntos[9] = 0;
				$sumatoria_puntos[10] = 0;
				$sumatoria_puntos[11] = 0;
				$sumatoria_puntos[12] = 0;//--
				$listado_npp_codigos = "";
				while (list($arreglo_plq_novedades[$contador_novedades][6], $arreglo_plq_novedades[$contador_novedades][7], $arreglo_plq_novedades[$contador_novedades][8],
						$arreglo_plq_novedades[$contador_novedades][9], $arreglo_plq_novedades[$contador_novedades][10], $arreglo_plq_novedades[$contador_novedades][11], $arreglo_plq_novedades[$contador_novedades][12],
						$arreglo_plq_novedades[$contador_novedades]["fecha_novedad"], $ndc_npp_codigo) = $this->_consultaAux->sigRegistroArreglo()) {
					$sumatoria_puntos[6] = $sumatoria_puntos[6] + $arreglo_plq_novedades[$contador_novedades][6];
					$sumatoria_puntos[7] = $sumatoria_puntos[7] + $arreglo_plq_novedades[$contador_novedades][7];
					$sumatoria_puntos[8] = $sumatoria_puntos[8] + $arreglo_plq_novedades[$contador_novedades][8];
					$sumatoria_puntos[9] = $sumatoria_puntos[9] + $arreglo_plq_novedades[$contador_novedades][9];
					$sumatoria_puntos[10] = $sumatoria_puntos[10] + $arreglo_plq_novedades[$contador_novedades][10];
					$sumatoria_puntos[11] = $sumatoria_puntos[11] + $arreglo_plq_novedades[$contador_novedades][11];
					$sumatoria_puntos[12] = $sumatoria_puntos[12] + $arreglo_plq_novedades[$contador_novedades][12];
					$contador_novedades++;

					if ($listado_npp_codigos == "")
						$listado_npp_codigos = "$ndc_npp_codigo";
					else
						$listado_npp_codigos .= ",$ndc_npp_codigo";
				}

				if ($contador_novedades > 0) { //Si existen novedades en el periodo liquidacion
//echo "<p>HAY NOVEDADES ESTE PERIODO</p>";
					$existe_retroactivo = TRUE; //Apenas haya novedad
					$sum_puntos_novedades = $sumatoria_puntos;
					$nomina_por_criterios = array();
					$retroactivo_novedad_criterios = array(); //Arreglo donde voy guardando los retroactivos por criterio de cada novedad

					for ($i = 0; $i < $contador_novedades; $i++) {
//echo "<p>CICLO $i</p>";
						//Borramos el registro de prenomina de la novedad anterior
						$SQL = "DELETE FROM liquidacionprenominatemporal
								WHERE		lqp_pel_codigo = '$this->_pel_codigo'
								AND			lqp_plq_codigo = '$this->_plq_codigo'";
						$this->_consultaAux->ejecutarConsulta($SQL);

						$dias_laborados_novedad = 0;
						$this->diasLaboradosPorNovedad($arreglo_plq_novedades[$i]["fecha_novedad"], $dias_laborados_novedad);
						$arreglo_plq_novedades[$i]["dias_laborados_novedad"] = round($dias_laborados_novedad);

						$sueldo_con_retroactivo = $this->sueldoConRetroactivoPorNovedad($sum_puntos_novedades);
						$this->_liquidacion->_plq_codigo = $plq_codigo;
						$this->_liquidacion->construirFechaLiquidacion($arreglo=array());

						if (!isset($this->_criteriosGrupo[$plq_codigo])) {
							$this->_criteriosGrupo[$plq_codigo]['criterios']=$this->obtenerCriteriosLiquidacionGrupo($plq_codigo);
							$SQL = "SELECT  plq_pcg_codigo
									FROM    periodoliquidacion
									WHERE   plq_codigo = $plq_codigo
									AND 	plq_registro_vigente";
							$this->_criteriosGrupo[$plq_codigo]['periodicidad']=$this->_consultaAux->obtenerCadenaResultadoConsulta($SQL, 1);
						}

						//Se obtienen los criterios de liquidaciòn del grupo de liquidacion para el periodo
						$criteriosGrupo = $this->_criteriosGrupo[$plq_codigo]['criterios'];
						$this->_liquidacion->_tipo_periodicidad=$this->_criteriosGrupo[$plq_codigo]['periodicidad'];
						$this->_liquidacion->criteriosGrupo = $this->_liquidacion->criteriosGrupoCopiaInicial = $criteriosGrupo;
						$this->_liquidacion->crearArreglosCriterios();
						$nombrecriterio = $this->_nombreCriterios[$this->_crl_codigo];

						$nombrecriterionombregrupo = strtolower($nombrecriterio).strtolower($this->_nombregrupo);
						$nombrecriterionombregrupo = explode(" ",$nombrecriterionombregrupo);

						$cadena = "";

						for ($j = 0; $j < sizeof($nombrecriterionombregrupo); $j++) {
							$cadena .= $nombrecriterionombregrupo[$j];
						}

						$arreglo_sueldo_basico['llave'] = $cadena;
						$arreglo_sueldo_basico['informacion'] = array("valor"=>$sueldo_con_retroactivo,"nombre" => $nombrecriterio,"tipo_variable"=>'ADICION');

						$this->_liquidacion->_grl_codigo = $this->_grl_codigo;
						$this->_liquidacion->_crl_codigo = $this->_crl_codigo;
						//=============
						//echo "<br> $nombrecriterio -- $this->_plq_codigo";

						if($nombrecriterio=='SUELDO BASICO'){
							if($this->_DEPURAR_RETROACTIVIDAD==1){
								echo "<BR><BR>LIQUIDANDO ".$this->_plq_codigo; 
								echo "<BR>SALARIO $sueldo_con_retroactivo";
							}
                        
							$this->_liquidacion->_criterio_omitir = $nombrecriterio;
							$this->_liquidacion->_valor_criterio_omitir = $sueldo_con_retroactivo;
							$DATOS_LIQUIDACION["lqp_pel_codigo"]      = $this->_pel_codigo;
							$DATOS_LIQUIDACION["lqp_plq_codigo"]      = $this->_plq_codigo;
							$DATOS_LIQUIDACION["lqp_grl_codigo"]      = $this->_grl_codigo;
							$DATOS_LIQUIDACION["lqp_crl_codigo"]      = $this->_crl_codigo;
							$DATOS_LIQUIDACION["lqp_valor_criterio"]  = $sueldo_con_retroactivo;
							$DATOS_LIQUIDACION["lqp_dias_liquidados"] = $this->dias_liquidados;
							$DATOS_LIQUIDACION["lqp_npp_codigo"]      = $variables_liquidacion["npp_codigo"]["valor"];
							if (!$this->_liquidacion->almacenarNomina($DATOS_LIQUIDACION)) {
								print "Ocurrió un error al almacenar los datos";
								$resp = FALSE;
							}
						}

						if (count($criteriosGrupo) > 0) {

							if ($this->_criteriosGrupo[$plq_codigo]['periodicidad'] != 2) {
								$var = $this->_liquidacion->ejecutarliquidacionperiodica($this->_pel_codigo, $verificarDiasAliquidar=TRUE,$arreglo_sueldo_basico);
							} else {
								$liquidarAnual=TRUE;
								if ($liquidarAnual) {
									$var = $this->_liquidacion->ejecutarliquidacionanual($this->_pel_codigo, $verificarDiasAliquidar=TRUE);
								}
							}
						}

						//Genero los valores en nómina solo una vez en el ciclo para ahorrar costos
						if ($i == 0)
							$this->nominaPorCriterios($nomina_por_criterios);

						$retroactivo_novedad_criterios[$i] = array();
						$this->retroactivoCriteriosNovedad($retroactivo_novedad_criterios[$i], $arreglo_plq_novedades[$i]["dias_laborados_novedad"], $nomina_por_criterios);

						//Si estamos en una segunda novedad hacia delante llevamos el acumulado
						if ($i > 0) {
							$arreglo_crl_codigos = array_keys($retroactivo_novedad_criterios[$i]);
							$cantidad_criterios = count($arreglo_crl_codigos);
							$dias_laborados_aplicados = $arreglo_plq_novedades[$i]["dias_laborados_novedad"] - $arreglo_plq_novedades[$i-1]["dias_laborados_novedad"];

							for ($j = 0; $j < $cantidad_criterios; $j++) {
								$retroactivo_novedad_criterios[$i][$arreglo_crl_codigos[$j]] = $retroactivo_novedad_criterios[$i][$arreglo_crl_codigos[$j]] / $arreglo_plq_novedades[$i]["dias_laborados_novedad"] * $dias_laborados_aplicados;
								$retroactivo_novedad_criterios[$i][$arreglo_crl_codigos[$j]] += $retroactivo_novedad_criterios[$i-1][$arreglo_crl_codigos[$j]];
							}
						}

						//$sum_puntos_novedades -= $arreglo_plq_novedades[$i]["puntos"];
						$sum_puntos_novedades[6] = $sum_puntos_novedades[6] - $arreglo_plq_novedades[$i][6];
						$sum_puntos_novedades[7] = $sum_puntos_novedades[7] - $arreglo_plq_novedades[$i][7];
						$sum_puntos_novedades[8] = $sum_puntos_novedades[8] - $arreglo_plq_novedades[$i][8];
						$sum_puntos_novedades[9] = $sum_puntos_novedades[9] - $arreglo_plq_novedades[$i][9];
						$sum_puntos_novedades[10] = $sum_puntos_novedades[10] - $arreglo_plq_novedades[$i][10];
						$sum_puntos_novedades[11] = $sum_puntos_novedades[11] - $arreglo_plq_novedades[$i][11];
						$sum_puntos_novedades[12] = $sum_puntos_novedades[12] - $arreglo_plq_novedades[$i][12];
					}/************************
					CIERRA EL CICLO FOR QUE RECORRE LAS NOVEDADES
					* ***********************/

					//Almacenamos todo el retroactivo acumulado por las novedades
					$resp = $this->almacenarRetroactivoAcumulado($retroactivo_novedad_criterios[$contador_novedades-1], $nomina_por_criterios, $ArregloAImprimir, $ArregloAImprimirValoresNegativos, $ArregloAImprimirValoresCero, $listado_npp_codigos);

					if (!$resp) {
						echo "<br><br>NO GUARDÓ EL RETROACTIVO ....<pre>";
						print_r($DATOS_LIQUIDACION);
					}

					//Asegurar que se sumen los puntos cuando cambie de mes y no sea por codigo de periodo de liquidacion
					$SQL = "SELECT plq_fecha_final_periodo FROM periodoliquidacion
							WHERE  plq_codigo = $this->_plq_codigo
							AND	   plq_registro_vigente";
					$fecha_final_plq_temp = $this->_consultaAux->obtenerCadenaResultadoConsulta($SQL);

					if ($fecha_final_plq_temp != $fecha_final_plq) {
						$this->_puntos_base[6] = $this->_puntos_base[6] + $sumatoria_puntos[6];
						$this->_puntos_base[7] = $this->_puntos_base[7] + $sumatoria_puntos[7];
						$this->_puntos_base[8] = $this->_puntos_base[8] + $sumatoria_puntos[8];
						$this->_puntos_base[9] = $this->_puntos_base[9] + $sumatoria_puntos[9];
						$this->_puntos_base[10] = $this->_puntos_base[10] + $sumatoria_puntos[10];
						$this->_puntos_base[11] = $this->_puntos_base[11] + $sumatoria_puntos[11];
						$this->_puntos_base[12] = $this->_puntos_base[12] + $sumatoria_puntos[12];
						$fecha_final_plq = $fecha_final_plq_temp;
					}

				} else if ($existe_retroactivo) { //Evita calculos innecesarios si no hay novedades en periodos anteriores
//echo "<p>-----------------NO HAY NOVEDADES ESTE PERIODO-----------------</p>";
					$sueldo_con_retroactivo = $this->sueldoConRetroactivo();
					$this->_liquidacion->_plq_codigo = $plq_codigo;
					$this->_liquidacion->construirFechaLiquidacion($arreglo=array());

					if (!isset($this->_criteriosGrupo[$plq_codigo])) {
						$this->_criteriosGrupo[$plq_codigo]['criterios']=$this->obtenerCriteriosLiquidacionGrupo($plq_codigo);
						$SQL = "SELECT  plq_pcg_codigo
								FROM    periodoliquidacion
								WHERE   plq_codigo = $plq_codigo
                                AND 	plq_registro_vigente";
						$this->_criteriosGrupo[$plq_codigo]['periodicidad']=$this->_consultaAux->obtenerCadenaResultadoConsulta($SQL);
					}

					//Se obtienen los criterios de liquidaciòn del grupo de liquidacion para el periodo
					$criteriosGrupo = $this->_criteriosGrupo[$plq_codigo]['criterios'];
					$this->_liquidacion->_tipo_periodicidad=$this->_criteriosGrupo[$plq_codigo]['periodicidad'];
					$this->_liquidacion->criteriosGrupo = $this->_liquidacion->criteriosGrupoCopiaInicial = $criteriosGrupo;
					$this->_liquidacion->crearArreglosCriterios();
					$nombrecriterio = $this->_nombreCriterios[$this->_crl_codigo];

					$nombrecriterionombregrupo = strtolower($nombrecriterio).strtolower($this->_nombregrupo);
					$nombrecriterionombregrupo = explode(" ",$nombrecriterionombregrupo);

					$cadena = "";

					for ($i = 0; $i < sizeof($nombrecriterionombregrupo); $i++) {
						$cadena .= $nombrecriterionombregrupo[$i];
					}

					$arreglo_sueldo_basico['llave'] = $cadena;
					$arreglo_sueldo_basico['informacion'] = array("valor"=>$sueldo_con_retroactivo,"nombre" => $nombrecriterio,"tipo_variable"=>'ADICION');

					$this->_liquidacion->_grl_codigo = $this->_grl_codigo;
					$this->_liquidacion->_crl_codigo = $this->_crl_codigo;
                    //=============
                    //echo "<br> $nombrecriterio -- $this->_plq_codigo";
                    
                    if ($nombrecriterio=='SUELDO BASICO') {
                        if ($this->_DEPURAR_RETROACTIVIDAD==1) {
                            echo "<BR><BR>LIQUIDANDO ".$this->_plq_codigo; 
                            echo "<BR>SALARIO $sueldo_con_retroactivo";
                        }
                        
                        $this->_liquidacion->_criterio_omitir = $nombrecriterio;
                        $this->_liquidacion->_valor_criterio_omitir = $sueldo_con_retroactivo;
                        $DATOS_LIQUIDACION["lqp_pel_codigo"]      = $this->_pel_codigo;
                        $DATOS_LIQUIDACION["lqp_plq_codigo"]      = $this->_plq_codigo;
                        $DATOS_LIQUIDACION["lqp_grl_codigo"]      = $this->_grl_codigo;
                        $DATOS_LIQUIDACION["lqp_crl_codigo"]      = $this->_crl_codigo;
                        $DATOS_LIQUIDACION["lqp_valor_criterio"]  = $sueldo_con_retroactivo;
                        $DATOS_LIQUIDACION["lqp_dias_liquidados"] = $this->dias_liquidados;
                        $DATOS_LIQUIDACION["lqp_npp_codigo"]      = $variables_liquidacion["npp_codigo"]["valor"];
                        if (!$this->_liquidacion->almacenarNomina($DATOS_LIQUIDACION)) {
							print "Ocurrió un error al almacenar los datos";
							$resp = FALSE;
                        }
                    }

					if (count($criteriosGrupo) > 0) {
					 /*$SQL = "SELECT  plq_pcg_codigo
                                  FROM    periodoliquidacion
                                  WHERE   plq_codigo = $this->_plq_codigo
                                          AND plq_registro_vigente";*/

						if ($this->_criteriosGrupo[$plq_codigo]['periodicidad'] != 2) {
							$var = $this->_liquidacion->ejecutarliquidacionperiodica($this->_pel_codigo, $verificarDiasAliquidar=TRUE,$arreglo_sueldo_basico);
						} else {
							$liquidarAnual=TRUE;
							if ($liquidarAnual) {
								$var = $this->_liquidacion->ejecutarliquidacionanual($this->_pel_codigo, $verificarDiasAliquidar=TRUE);
							}
						}
					}

					$resp = $this->almacenarRetroactivo($ArregloAImprimir, $ArregloAImprimirValoresNegativos, $ArregloAImprimirValoresCero);

					if (!$resp) {
						echo "<br><br>NO GUARDÓ EL RETROACTIVO ....<pre>";
						print_r($DATOS_LIQUIDACION);
					}
				}//Termina el if de si existen novedades
            }//foreach
            //echo "<br>----->FIN FOREACH ".$this->hora();

            //Si no existió ninguna novedad en el periodo retroactivo
            if (!$existe_retroactivo)
				$resp = TRUE;
        }
        else
        {
            $error = new error("ERROR","ERROR878");
            $mesg = $error->retornarMensajeError();
            eval( "\$mesg = \"$mesg\";" );
            print "<center>$mesg</center><br>";
            $resp = TRUE;
        }

        unset($arreglo_plq_cod);
        unset($arreglo_plq_cod);
        unset($sueldo_con_retroactivo);
        unset($criteriosGrupo);
        unset($arreglo_sueldo_basico);
        unset($DATOS_LIQUIDACION);
        $this->eliminarValoresDeCriteriosAjenosAVacacionesDePeriodosForaneosAlPeriodoRetroactivo();
        return  $resp;
    }    
    

     /**
     * Retorna el complemento where que permite omitir a los pelcodigos que yá se les ha liquidado
     * excedente por nivelación, es decir, que ya aparece la novedad de cambio de salario registrada en
     * liquidacionnomina.
     *
     * @author      Alexander Varela Galvis
     * @access      public
     * @copyright   OIT, Universidad del Valle 2.008
     * @since       2008-06-10
     * @return      string
     */
    function obtenerPelCodigosNoLiquidados($prt_tipo='NIVELACION',$crl_codigo='104'){
       return "  OR lra_pel_codigo in ( SELECT distinct lra_pel_codigo
                                        FROM   liquidacionretroactividad,periodoretroactivo
                                        WHERE  lra_prt_codigo=prt_codigo
                                        AND    prt_tipo='$prt_tipo'
                                        AND    lra_prt_codigo!=$this->_prt_codigo
                                        --AND    lra_prt_codigo!=45
                                        AND    lra_npp_codigo_cambio_salario NOT IN (SELECT lqd_npp_codigo
                                        					     FROM   liquidacionnomina
                                        					     WHERE  lqd_crl_codigo='$crl_codigo'
                                        					     AND    lqd_registro_vigente))";


    }



    /**
     * Retorna el complemento where que permite omitir a los pelcodigos que yá se les ha liquidado
     * excedente por nivelación, es decir, que ya aparece la novedad de cambio de salario registrada en
     * liquidacionnomina.
     *
     * @author      Alexander Varela Galvis
     * @access      public
     * @copyright   OIT, Universidad del Valle 2.008
     * @since       2008-06-10
     * @return      string
     */
     function obtenerWhereOmitirPelcodigos($criterio='EXCEDENTE POR NIVELACION',$prt_tipo='NIVELACION'){

          $complemento=($this->_origen_datos=='Pagados')?"":"NOT";
          
          //$complemento_npp_codigo="AND    lra_npp_codigo_cambio_salario=lqd_npp_codigo";
          if($prt_tipo=='PORCENTAJE'){
            $complemento_npp_codigo="";
          }

          return "  AND lra_pel_codigo $complemento in ( SELECT distinct lqd_pel_codigo
                                              FROM   liquidacionnomina,criterioliquidacion,
                                                     liquidacionretroactividad,periodoretroactivo,
                                                     retroactividadespagadas
                                              WHERE  lqd_crl_codigo=crl_codigo
                                              AND    crl_nombre='$criterio'
                                              AND    lqd_registro_vigente
                                              AND    crl_registro_vigente
                                              AND    lra_pel_codigo=lqd_pel_codigo
                                              AND 	 lra_codigo = rtp_lra_codigo
                                              AND    lqd_plq_codigo=prt_plq_codigo
                                              AND    prt_tipo='$prt_tipo'
                                              AND    prt_codigo=lra_prt_codigo
                                              AND    lra_prt_codigo=$this->_prt_codigo
                                              )";

    }


    /**
     * Elimina la liquidaciones anteriores en la tabla de liquidacionretroactividad
     *
     * @author      Maricel rubio
     * @access      public
     * @copyright   OIT, Universidad del Valle 2.004
     * @since       2004-11-30
     */

    function borrarTablaLiquidacionRetroactivo ($personaOgrupo)
    {
        $delete_con_retro_sin_nomina  = $this->obtenerPelCodigosConRetroLiquidadaPeroNoPagada($personaOgrupo);
        switch($personaOgrupo)
        {
            case "PERSONA":
                $resto_where           ="lra_pel_codigo='$this->_pel_codigo'";
                $resto_where_negativos ="lrn_pel_codigo='$this->_pel_codigo'";
            break;

            case "GRUPO":
                //$arreglo = implode (" OR lra_plq_codigo_liquidado = ", $this->arreglo_plq_retroactivo);

                $where_retro_nivelacion="";
                $resto_where_retro_nivelacion="";
                //Si el tipo de retro es nivelacion, no se pueden borrar las retroactividades que ya fueron
                //pagadas, es decir, que ya aparecen en liquidacionnomina.
                if($this->_prt_tipo=='NIVELACION'){
                      $where_retro_nivelacion = $this->obtenerWhereOmitirPelcodigos('EXCEDENTE POR NIVELACION','NIVELACION');
                      // ejecutar la retro por nivelación, no implica que se realice dicho pago
                      // en la liquidación de nómina, si a un individuo no se le paga la nivelación debe tenerse
                      // en cuenta para la próxima liquidación de retro por nivelación.
                      $resto_where_retro_nivelacion = $this->obtenerPelCodigosNoLiquidados('NIVELACION','104');
                }

                if($this->_prt_tipo=='CAMBIOSALARIO'){
                      $where_retro_nivelacion = $this->obtenerWhereOmitirPelcodigos('RETROACTIVO','CAMBIOSALARIO');
                      // ejecutar la retro por nivelación, no implica que se realice dicho pago
                      // en la liquidación de nómina, si a un individuo no se le paga la nivelación debe tenerse
                      // en cuenta para la próxima liquidación de retro por nivelación.
                      $resto_where_retro_nivelacion = $this->obtenerPelCodigosNoLiquidados('CAMBIOSALARIO','111');
                }


                $resto_where = "lra_pel_codigo IN (
                                        SELECT  pel_codigo
                                        FROM    personaliquidacion
                                        WHERE   pel_grl_codigo='$this->_grl_codigo'
                                     )
                    $where_retro_nivelacion";
                    
                $resto_where_negativos =" lrn_pel_codigo=personaliquidacion.pel_codigo
                                          AND personaliquidacion.pel_grl_codigo='$this->_grl_codigo'";
               
            break;
        }

        $columnas= array(
                         "lra_codigo","lra_pel_codigo","lra_prt_codigo",
                         "lra_crl_codigo_liquidado","lra_plq_codigo_liquidado","lra_npp_codigo_liquidado",
                         "lra_valor_liquidado","lra_usu_login","lra_fecha_modificacion","lra_registro_vigente"
                        );

        $DELETE = "(lra_prt_codigo = '$this->_prt_codigo' AND $resto_where) $resto_where_retro_nivelacion";

        //echo "<br> <pre> $DELETE2";

        $this->_tabla_liquidacion->establecerColumnas($columnas);
        $this->_tabla_liquidacion->deleteReal($DELETE);
        $this->_tabla_liquidacion->deleteReal($delete_con_retro_sin_nomina);
        
        
        $columnas= array("lrn_codigo","lrn_pel_codigo","lrn_prt_codigo","lra_crl_codigo","lrn_valor_criterio",
                         "lrn_usu_login","lrn_fecha_modificacion","lrn_registro_vigente");
        $DELETE = "(lrn_prt_codigo = '$this->_prt_codigo' AND $resto_where_negativos)";        
        $this->_tabla_negativos->establecerColumnas($columnas);
        $this->_tabla_negativos->deleteReal($DELETE);       
        
    }

      /**
     * Devuelve la cada delete a ejecutar, de acuerdo a las liquidaciones de retroactividad que no han sido pagadas,
     * es decir, que aparecen en liquidacionretroactividad pero no en liquidacionomina.
     * @author      Alexander Varela Galvis
     * @access      public
     * @copyright   OITEL, Universidad del Valle 2.009
     * @since       2009-08-14
     * @return      string
     */
    function obtenerPelCodigosConRetroLiquidadaPeroNoPagada($personaOgrupo){
          switch($this->_prt_tipo){
              case 'NIVELACION':
                $crl_codigo=104;
              break;
              case 'CAMBIOSALARIO':
                $crl_codigo=111;
              break;
              default:
                $crl_codigo=41;
          }

          if($personaOgrupo=='PERSONA'){
              $resto_where_retro="AND lra_pel_codigo='$this->_pel_codigo'";
              $resto_where_nomina="AND lqd_pel_codigo='$this->_pel_codigo'";
          }

          /*return "lra_codigo in (  SELECT lra_codigo
                              			FROM   liquidacionretroactividad,periodoretroactivo
                              			WHERE  lra_prt_codigo=prt_codigo
                              			AND    prt_tipo='$this->_prt_tipo'
                              			--AND    prt_codigo!=45
                              			AND    lra_npp_codigo_cambio_salario NOT IN (SELECT lqd_npp_codigo
                                                                					      FROM   liquidacionnomina
                                                                					      WHERE  lqd_crl_codigo=$crl_codigo
                                                                					      $resto_where_nomina
                                                                					      AND    lqd_registro_vigente))
                                    $resto_where_retro";*/

		return "lra_codigo NOT IN (	SELECT rtp_lra_codigo
									FROM   retroactividadespagadas)
				$resto_where_retro
				AND lra_prt_codigo = '$this->_prt_codigo'";
    }


    /**
     * Devuelve el valor del criterio si este se pagó adelantado.
     * Obs: Un criterio anual como bsp o p.antigüedad, se paga adelantado si este cae dentro del periodo
     * de disfrute de vacaciones
     * @author      Alexander Varela Galvis
     * @access      public
     * @copyright   OITEL, Universidad del Valle 2.009
     * @since       2009-05-14
     * @return      boolean
     */
    function consultarPagoAdelantadoDelCriterio($lqp_crl_codigo){
        $consulta = new Consulta($this->_bd);
        $consultaAux1 = new Consulta($this->_bd);
        switch($lqp_crl_codigo){
          case 29://bsqp
            $tfl_codigo=1;
          break;
          case 6://prima de antigüedad
            $tfl_codigo=2;
          break;
        }

        $sql= "SELECT fpl_fecha_criterio
               FROM   novedadporpersona,fechasparaliquidarcriterios
               WHERE  npp_pel_codigo=$this->_pel_codigo
               AND    npp_codigo=fpl_npp_codigo
               AND    fpl_tfl_codigo=$tfl_codigo
               AND    novedadaterminadoflujo(npp_codigo)
               AND NOT npp_anulada
               AND    fpl_registro_vigente
               AND    npp_registro_vigente
               ORDER  BY npp_fecha_novedad DESC,npp_fecha_modificacion DESC
               LIMIT  1";
        $fecha=$consulta->obtenerCadenaResultadoConsulta($sql);
        $sql= "SELECT PeriodoIncluyePagoAnual($this->_plq_codigo,'$fecha')";
        
        if($consulta->obtenerCadenaResultadoConsulta($sql)=='t'){
                $yaSePago="";
                $sql="  SELECT TRUE
                        FROM   liquidacionretroactividad
                        WHERE  lra_pel_codigo=$this->_pel_codigo
                        AND    lra_crl_codigo_liquidado=$lqp_crl_codigo
                        AND    lra_prt_codigo=$this->_prt_codigo
                        AND    lra_registro_vigente";
                $yaSePago=$consulta->obtenerCadenaResultadoConsulta($sql);

                if($yaSePago!="t"){echo "<p>ya se pago</p>";
                      if($this->_herramientas->tablaExiste('valorespagados')){
                          $consultaAux1->ejecutarConsulta("DROP TABLE valorespagados;");
                      }

                     $SQL = " SELECT  lqd_valor_criterio,lqd_plq_codigo,plq_fecha_inicio_periodo
                              FROM    liquidacionnomina,periodoliquidacion
                              WHERE   lqd_pel_codigo = $this->_pel_codigo AND
                                      lqd_crl_codigo = $lqp_crl_codigo AND
                                      lqd_plq_codigo in (
                                              SELECT vac_plq_codigo
                                              FROM   novedadporpersona,vacaciones
                                              WHERE  npp_pel_codigo=$this->_pel_codigo
                                              AND    npp_codigo=vac_npp_codigo
                                              AND NOT npp_anulada
                                              AND    novedadaterminadoflujo(npp_codigo)
                                              AND    npp_registro_vigente
                                              AND    vac_registro_vigente
                                              AND    to_char(date '$fecha', 'MM-DD')
                                              BETWEEN to_char(npp_fecha_novedad, 'MM-DD')
                                              AND     to_char(vac_fecha_finalizacion_novedad, 'MM-DD')
                                              AND    diasdesolapamiento(npp_fecha_novedad,vac_fecha_finalizacion_novedad,date '$this->_prt_fecha_inicio', date '$this->_prt_fecha_final') > 1
                                              ) AND
                                       lqd_plq_codigo=plq_codigo AND
                                       plq_registro_vigente";
                      $consultaAux1->ejecutarConsulta($SQL);

                      if($consultaAux1->numFilas()>0){
                          list($pagadoAdelantado,$plq_codigo,$plq_fecha_inicio_periodo)=$consultaAux1->sigRegistroArreglo();
                          $SQL="  SELECT  lqd_valor_criterio
                                  FROM    liquidacionnomina,pagosporunasolavez,periodoliquidacion
                                  WHERE   lqd_registro_vigente
                                          AND lqd_pel_codigo = '$this->_pel_codigo'
                                          AND lqd_npp_codigo=puv_npp_codigo
                                          AND puv_plq_codigo_origen = plq_codigo
                                          AND plq_fecha_inicio_periodo>='$plq_fecha_inicio_periodo'
                                          AND puv_crl_codigo=$lqp_crl_codigo
                                          AND puv_registro_vigente
                                          AND plq_registro_vigente";
                          $PagoPorUnaVez=$consultaAux1->obtenerCadenaResultadoConsulta($SQL);
                      }

                      $total=$pagadoAdelantado+$PagoPorUnaVez;

                      $sql = "SELECT  $total";
                     unset($consultaAux1);
                     return $sql;
                 }else{
                  unset($consultaAux1);
                  return FALSE;
                 }
         }else{
                 unset($consultaAux1);
                 return FALSE;
         }
         unset($consultaAux1);

    }//consultarPagoAdelantadoDelCriterio

    /**
     * Si un periodo no tuvo liquidación porque el empleado se encontraba en vacaciones y las vacaciones fueron pagadas
     * fuera del periodo retroactivo, hay que pagarle los excedentes del periodo no pagado.
     * @author      Alexander Varela Galvis
     * @access      public
     * @copyright   OIT, Universidad del Valle 2.009
     * @since       2009-10-29
     * @return      void
     */
    function almacenarExcedenteSueldoBasicoParaPeriodoNoLiquidado(&$ArregloAImprimir){    
        $sql=" SELECT npp_codigo,vac_fecha_finalizacion_novedad
               FROM   liquidacionnomina,vacaciones,
                      novedadporpersona,periodoliquidacion
               WHERE  lqd_pel_codigo=$this->_pel_codigo
               AND    lqd_crl_codigo=14
               AND    lqd_npp_codigo=npp_codigo
               AND    lqd_pel_codigo=npp_pel_codigo
               AND    npp_codigo=vac_npp_codigo
               AND not npp_anulada
               AND    vac_plq_codigo not in ($this->_cadena_arreglo_plq_cod)
               AND    plq_codigo = $this->_plq_codigo
               AND    novedadaterminadoflujo(npp_codigo)
               AND    diasdesolapamiento(npp_fecha_novedad,vac_fecha_finalizacion_novedad,plq_fecha_inicio_periodo,plq_fecha_final_periodo) > 1
               AND    npp_fecha_novedad<'$this->_prt_fecha_inicio'
               AND    npp_registro_vigente
               AND    plq_registro_vigente
               AND    lqd_registro_vigente
               AND    vac_registro_vigente";
        $this->_consultaAux3->ejecutarConsulta($sql);
        list($npp_codigo,$vac_fecha_finalizacion_novedad)=$this->_consultaAux3->sigRegistroArreglo();

        $crl_codigo       = 73;
        $nombre_criterio  = "SUELDO BASICO";
        if($npp_codigo){
            switch($this->_grl_codigo){
                  case '3':
                  case '5':
                  case '6':
                    $dias_periodo=15;
                  break;
            }
    
            $Retro=$this->_sueldoNuevoDeAcuerdoAlPeriodo-$this->_sueldoAnteriorDeAcuerdoAlPeriodo;
    
            if($Retro>0){
                $this->insertarCriterio($ArregloAImprimir, $Retro, $this->_plq_codigo, $npp_codigo, $dias_periodo, $crl_codigo,$nombre_criterio);
            }//if($RetroVacaciones>0)
        }
    }



	/**
     * Halla las liquidaciones de nomina en el periodo de liquidación por cada criterio
     * nominaPorCriterios
     * @author      Cristhian David Marín
     * @access      public
     * @copyright   OITEL, Universidad del Valle 2014
     * @since       2014
     * @return      void
     */
	function nominaPorCriterios(&$nomina_por_criterios)
	{
		//Consulta todas las liquidaciones de la persona en un periodo de liquidación en prenomina con la nueva nivelación
        $SQL = "SELECT   lqp_crl_codigo, lqp_valor_criterio, lqp_npp_codigo
                FROM     liquidacionprenominatemporal
                WHERE    lqp_pel_codigo = '$this->_pel_codigo'
                AND 	 lqp_plq_codigo = '$this->_plq_codigo'
                ORDER BY lqp_codigo";

        $consultaAux = new Consulta($this->_bd);
        $consultaAux->ejecutarConsulta($SQL);

        $i=0;

		//Comienza recorrer los registros de liquidación en el periodo
        if($consultaAux->numFilas() > 0)
        {
            while(list($lqp_crl_codigo, $lqp_valor_criterio, $lqp_npp_codigo) = $consultaAux->sigRegistroArreglo())
            {
                $consultaAux1 = new Consulta($this->_bd);

//echo "<br><br>======= criterio $lqp_crl_codigo --- lqp_valor_criterio -- $lqp_valor_criterio  -- periodo $this->_plq_codigo ============ novedad $this->lqp_npp_codigo";

                unset($where_liquidacionnomina);
                unset($where_liquidacionretro);
                if ($lqp_npp_codigo != "")
                {
                    $where_liquidacionnomina = "AND lqd_npp_codigo = '$lqp_npp_codigo'";
                    $where_liquidacionretro = "AND lra_npp_codigo_liquidado = '$lqp_npp_codigo'";
                }

				//Dependiendo del criterio, se obtienen el valor y los días liquidados guardados en nómina
                switch($lqp_crl_codigo){
                  case '60':
                      //Si es un px1vez de criterios salariales que debieron 
                      //pagarse dentro del periodo retroactivo, aplica retroactividad.
                      $criteriosBaseSS=$this->_liquidacion->_seguridad_social->_criteriosBaseSeguridadSocialPension;
                      $SQL = "SELECT  puv_valor,lqd_dias_liquidados
                              FROM    pagosporunasolavez,liquidacionnomina
                              WHERE   puv_npp_codigo=$lqp_npp_codigo
                              AND     puv_registro_vigente
                              AND     puv_plq_codigo_origen in ($this->_cadena_arreglo_plq_cod)
                              AND     puv_crl_codigo in ($criteriosBaseSS)
                              AND     lqd_pel_codigo = '$this->_pel_codigo'
                              AND     lqd_plq_codigo = '$this->_plq_codigo'
                              AND     lqd_crl_codigo = '$lqp_crl_codigo'
                              AND     lqd_npp_codigo = puv_npp_codigo";
                              
                      $consultaAux1->ejecutarConsulta($SQL);
                      if($consultaAux1->numFilas()<=0){
                            continue;
                      }
                      
                  break;
                  case '6':
                  case '29':
                     $SQL = "SELECT  lqd_valor_criterio,lqd_dias_liquidados
                             FROM    liquidacionnomina
                             WHERE   lqd_registro_vigente
                                     AND lqd_pel_codigo = '$this->_pel_codigo'
                                     AND lqd_plq_codigo = '$this->_plq_codigo'
                                     AND lqd_crl_codigo = '$lqp_crl_codigo'
                                     $where_liquidacionnomina";
                     $consultaAux1->ejecutarConsulta($SQL);
                     if($consultaAux1->numFilas()<=0){
                       if($sql=$this->consultarPagoAdelantadoDelCriterio($lqp_crl_codigo)){
                           $consultaAux1->ejecutarConsulta($sql);
                       }
                     }  
                  break;
                
                  default:

                    $SQL = "SELECT  lqd_valor_criterio,lqd_dias_liquidados
                            FROM    liquidacionnomina
                            WHERE   lqd_registro_vigente
                                    AND lqd_pel_codigo = '$this->_pel_codigo'
                                    AND lqd_plq_codigo = '$this->_plq_codigo'
                                    AND lqd_crl_codigo = '$lqp_crl_codigo'
                                    $where_liquidacionnomina";
                    $consultaAux1->ejecutarConsulta($SQL);
                    if($consultaAux1->numFilas()<1 && $lqp_crl_codigo=='73'){
                      switch($this->_grl_codigo){
                          case '1':
                          case '2':
                          case '3':
                          case '5':
                          case '6':
                              $this->almacenarExcedenteSueldoBasicoParaPeriodoNoLiquidado(&$ArregloAImprimir);
                          break;
                      }
                    }
                }



				//Si encontró liquidaciones por cada criterio anteriormente
                if($consultaAux1->numFilas() > 0)
                {
                    while(list($lqd_valor_criterio,$lqd_dias_liquidados) = $consultaAux1->sigRegistroArreglo())
                    {
                       //Si el criterio es sueldo básico o sus afines, verifico si los días liquidados son
                       //menos de los correspondientes a los días de nómina del periodo, si es así, se verifica
                       //si esto se debe al solapamiento del disfrute de vacaciones pero para aquellas que fueron pagadas
                       //antes del periodo retroactivo. Si es así se debe pagar retro al periodo de liquidación completo.
                       switch($lqp_crl_codigo){
                            case '26':
                            case '72':
                            case '73':
                                if($lqd_dias_liquidados!=$this->_dias_nomina_del_periodo){
                                    if($this->diasSolapadosConDisfruteDeVacaciones()){
                                        $lqd_valor_criterio=($lqd_valor_criterio/$lqd_dias_liquidados)*$this->_dias_nomina_del_periodo;
                                    }
                                }
                            break;
                        }

						$nomina_por_criterios[$lqp_crl_codigo]["valor"] = $lqd_valor_criterio;
						$nomina_por_criterios[$lqp_crl_codigo]["dias_liquidados"] = $lqd_dias_liquidados;
						$nomina_por_criterios[$lqp_crl_codigo]["npp_codigo"] = $lqp_npp_codigo;
//echo "<p>dias liquidados: $lqd_dias_liquidados</p>";
					}

				}
			}
		}
	}



	/**
     * Hace la diferencia entre los datos de las tablas liquidacionnomina y liquidacionprenominatemporal por cada criterio
     * liquidacionretroactividad
     * @author      Cristhian David Marín
     * @access      public
     * @copyright   OITEL, Universidad del Valle 2014
     * @since       2014
     * @return      void
     */
    function retroactivoCriteriosNovedad (&$retroactivo_novedad_criterios, &$dias_laborados_novedad, &$nomina_por_criterios)
    {
        $resp = TRUE;

        //Consulta todas las liquidaciones de la persona en un periodo de liquidación en prenomina con la nueva nivelación
        $SQL = "SELECT   lqp_crl_codigo, lqp_valor_criterio, lqp_npp_codigo
                FROM     liquidacionprenominatemporal
                WHERE    lqp_pel_codigo = '$this->_pel_codigo'
                AND 	 lqp_plq_codigo = '$this->_plq_codigo'
                ORDER BY lqp_codigo";

        $consultaAux = new Consulta($this->_bd);
        $consultaAux->ejecutarConsulta($SQL);

        $i=0;

		//Comienza recorrer los registros de liquidación en el periodo
        if ($consultaAux->numFilas() > 0)
        {
            while (list($lqp_crl_codigo, $lqp_valor_criterio, $lqp_npp_codigo) = $consultaAux->sigRegistroArreglo())
            {
				//Se obtiene el valor retroactivo a pagar aplicando la diferencia entre el valor de prenómina y el de nómina
				$valor_retroactivo_a_pagar = $lqp_valor_criterio - $nomina_por_criterios[$lqp_crl_codigo]["valor"];
				$lqd_valor_criterio = $nomina_por_criterios[$lqp_crl_codigo]["valor"];
//echo "<p>retroactivo: $valor_retroactivo_a_pagar = prenomina: $lqp_valor_criterio - nomina : ".$nomina_por_criterios[$lqp_crl_codigo]["valor"]."</p>";
				if ($dias_laborados_novedad <= $nomina_por_criterios[$lqp_crl_codigo]["dias_liquidados"])
					$valor_retroactivo_a_pagar = $valor_retroactivo_a_pagar / $nomina_por_criterios[$lqp_crl_codigo]["dias_liquidados"] * $dias_laborados_novedad;
//echo "<p>retro a pagar : $valor_retroactivo_a_pagar</p>";
				$retroactivo_novedad_criterios[$lqp_crl_codigo] = $valor_retroactivo_a_pagar;
//echo "<p>dias laborados: $dias_laborados_novedad</p>";
				if ($this->_DEPURAR_RETROACTIVIDAD==1) {
					echo "<br>Criterio $lqp_crl_codigo";
					echo "<br>valor_retroactivo_a_pagar $valor_retroactivo_a_pagar = lqp_valor_criterio-lqd_valor_criterio => $lqp_valor_criterio-$lqd_valor_criterio ";
				}

            }
        }

        if ($this->_tipo!='PERSONA') {
          unset($ArregloAImprimirValoresCero);
          unset($ArregloAImprimirValoresNegativos);
          unset($ArregloAImprimir);
        }

        $consultaAux->liberarMemoria();
        unset($consultaAux);
        unset($DATOS_RETROACTIVOS);
        return $resp;
    }



	/**
     * Almacena el retroactivo calculado en el periodo de liquidacion
     * liquidacionretroactividad
     * @author      Cristhian David Marín
     * @access      public
     * @copyright   OITEL, Universidad del Valle 2014
     * @since       2014
     * @return      void
     */
	function almacenarRetroactivoAcumulado (&$retroactivo_novedad_criterios, &$nomina_por_criterios, &$ArregloAImprimir, &$ArregloAImprimirValoresNegativos, &$ArregloAImprimirValoresCero, &$listado_npp_codigos)
	{
		$resp = TRUE;
		$arreglo_crl_codigos = array_keys($retroactivo_novedad_criterios);
		$cantidad_criterios = count($arreglo_crl_codigos);

		for($i = 0; $i < $cantidad_criterios; $i++) {
			$criterio_codigo = $arreglo_crl_codigos[$i];

			if ($nomina_por_criterios[$criterio_codigo]["valor"] > 0) {

				switch($this->_prt_tipo){
					case "NIVELACION":
						$crl_codigo=104;
					break;

					default;
						$crl_codigo=41;
				}

				//Busca si se ha realizado tareas de retroactividad anteriores y se restan a la actual
				$SQL = "SELECT  sum(lra_valor_liquidado)
						FROM    liquidacionretroactividad,liquidacionnomina,periodoretroactivo
						WHERE   lra_pel_codigo = '$this->_pel_codigo'
						AND   lra_plq_codigo_liquidado = '$this->_plq_codigo'
						AND   lra_crl_codigo_liquidado = '$criterio_codigo'
						AND   lra_registro_vigente $where_liquidacionretro
						AND   lra_prt_codigo=prt_codigo
						AND   prt_plq_codigo=lqd_plq_codigo
						AND   lra_pel_codigo=lqd_pel_codigo
						AND   lqd_crl_codigo in (41,104,111)
						AND   lqd_registro_vigente
						AND   lra_registro_vigente";
				$valores_retroactivos_anteriores = $this->_consultaAux2->obtenerCadenaResultadoConsulta($SQL);
				$valor_retroactivo_a_pagar 		 = $retroactivo_novedad_criterios[$criterio_codigo];
				$valor_retroactivo_a_pagar       = $valor_retroactivo_a_pagar - $valores_retroactivos_anteriores;

				$retroactivo_novedad_criterios[$criterio_codigo] = $valor_retroactivo_a_pagar;
//if($valores_retroactivos_anteriores > 0){echo "<p>retroactivos anteriores: $valores_retroactivos_anteriores</p>";echo "<p>retro final: $valor_retroactivo_a_pagar</p>";}
				if($this->_DEPURAR_RETROACTIVIDAD==1){
					echo "<br>lqd_valor_criterio=$lqd_valor_criterio ";
					echo "<br>valores_retroactivos_anteriores $valores_retroactivos_anteriores";
					echo "<br>valor_retroactivo_a_pagar Despues $valor_retroactivo_a_pagar";
				}

				if ($valor_retroactivo_a_pagar > 1) {//Es más costoso el trámite admitivo para pagar un peso.
					if ($lqp_crl_codigo == "29" || $lqp_crl_codigo == "35" || $lqp_crl_codigo == "36" || $lqp_crl_codigo == "31"
						|| $lqp_crl_codigo == "32" || $lqp_crl_codigo == "33" || $lqp_crl_codigo == "79")
					{
						if ($this->_grl_codigo == "11" || $this->_grl_codigo == "12") {
							$SQL = "SELECT obtenerLiquidadoPorCriterioOtrosPeriodos($this->_pel_codigo,$this->_plq_codigo,$lqp_crl_codigo)";
							$pagos_otros_periodos = $consultaAux2->obtenerCadenaResultadoConsulta($SQL);
							$valor_retroactivo_a_pagar = $valor_retroactivo_a_pagar - $pagos_otros_periodos;
						}
					}

					if ($valor_retroactivo_a_pagar > 0) {

						$DATOS_RETROACTIVOS["lra_pel_codigo"]            		= $this->_pel_codigo;
						$DATOS_RETROACTIVOS["lra_prt_codigo"]            		= $this->_prt_codigo;
						$DATOS_RETROACTIVOS["lra_valor_liquidado"]       		= round($valor_retroactivo_a_pagar);
						$DATOS_RETROACTIVOS["lra_crl_codigo_liquidado"]  		= $criterio_codigo;
						$DATOS_RETROACTIVOS["lra_plq_codigo_liquidado"]  		= $this->_plq_codigo;
						$DATOS_RETROACTIVOS["lra_npp_codigo_liquidado"]  		= $listado_npp_codigos;//$nomina_por_criterios[$criterio_codigo]["npp_codigo"];
						$DATOS_RETROACTIVOS["lra_dias_liquidados"]       		= $this->dias_liquidados;
						$DATOS_RETROACTIVOS["lra_npp_codigo_cambio_salario"]  	= $this->_arreglo_novedad[$this->_pel_codigo];

						//Se almacena en la Base de Datos la liquidaciòn del Retroactivo
//echo "<br> <pre>";print_r($DATOS_RETROACTIVOS);
						if (!$this->insertar($DATOS_RETROACTIVOS)) {
							$this->_bd->abortarTransaccion();
							$error = new error("AVISO","AVISO4");
							$mesg = $error->retornarMensajeError();
							print "<center>$mesg</center>";
							$resp = FALSE;
						} else {
							if ($lqp_crl_codigo!=20 && $lqp_crl_codigo!=21) {
								if ($this->_tipo=='PERSONA') {
									$SQL = "SELECT crl_nombre FROM criterioliquidacion WHERE  crl_codigo = '$criterio_codigo'";
									$crl_nombre = $this->_consultaAux2->obtenerCadenaResultadoConsulta($SQL);
									$valor = $ArregloAImprimir[$this->_pel_codigo]["Criterios"];
									$ArregloAImprimir[$this->_pel_codigo]["Criterios"][$crl_nombre] += $DATOS_RETROACTIVOS['lra_valor_liquidado'];
								}
							}

							if ($lqp_crl_codigo==20 || $lqp_crl_codigo==21) {
								$this->registrarIngresoBase($lqp_crl_codigo);
								if ($this->_tipo=='PERSONA') {
									$SQL = "SELECT crl_nombre FROM criterioliquidacion WHERE  crl_codigo = '$criterio_codigo'";
									$crl_nombre = $consultaAux2->obtenerCadenaResultadoConsulta($SQL);
									$ArregloAImprimir[$this->_pel_codigo]["Criterios"][$crl_nombre] += $DATOS_RETROACTIVOS['lra_valor_liquidado'];
								}
							}
						}
					} else {
						$valor = round($valor_retroactivo_a_pagar);
						$datos['lrn_crl_codigo']=$criterio_codigo;
						$datos['lrn_valor_criterio']=$valor;
						$this->insertarValorNegativoOCero($datos);
					}
				}//if ($valor_retroactivo_a_pagar > 1
				else {
					$valor = round($valor_retroactivo_a_pagar);
					$datos['lrn_crl_codigo']=$criterio_codigo;
					$datos['lrn_valor_criterio']=$valor;
					$this->insertarValorNegativoOCero($datos);
				}
			}//if ($lqd_valor_criterio > 0)
		}

		if($this->_tipo!='PERSONA'){
			unset($ArregloAImprimirValoresCero);
			unset($ArregloAImprimirValoresNegativos);
			unset($ArregloAImprimir);
        }

        return $resp;
	}



    /**
     * Hace la diferencia entre los datos de las tablas liquidacionnomina y liquidacionprenominatemporal para ingresar el valor en la tabla
     * liquidacionretroactividad
     * @author      Cristhian David Marín
     * @access      public
     * @copyright   OITEL, Universidad del Valle 2014
     * @since       2014
     * @return      void
     */
    function almacenarRetroactivo (&$ArregloAImprimir, &$ArregloAImprimirValoresNegativos, &$ArregloAImprimirValoresCero)
    {
        $resp = TRUE;

        //Consulta todas las liquidaciones de la persona en un periodo de liquidación en prenomina con la nueva nivelación
        $SQL = "SELECT  lqp_crl_codigo, lqp_valor_criterio, lqp_npp_codigo
                FROM    liquidacionprenominatemporal
                WHERE   lqp_pel_codigo = '$this->_pel_codigo'
                        AND lqp_plq_codigo = '$this->_plq_codigo'
                ORDER BY lqp_codigo
             ";

        $consultaAux = new Consulta($this->_bd);
        $consultaAux->ejecutarConsulta($SQL);

        $i=0;

		//Comienza recorrer los registros de liquidación en el periodo
        if($consultaAux->numFilas() > 0)
        {
            while(list($lqp_crl_codigo, $lqp_valor_criterio, $lqp_npp_codigo) = $consultaAux->sigRegistroArreglo())
            {
                $consultaAux1 = new Consulta($this->_bd);

//echo "<br><br>======= criterio $lqp_crl_codigo --- lqp_valor_criterio -- $lqp_valor_criterio  -- periodo $this->_plq_codigo ============ novedad $this->lqp_npp_codigo";

                //Para los pagos y descuentos por supervivencia
                /*
                 if ($lqp_crl_codigo == 87 || $lqp_crl_codigo == 88 || $lqp_crl_codigo == 90)
                {
                    $SQL = "SELECT  pms_crl_codigo, pms_ten_codigo, pms_valor, pms_plq_codigo_liquidado
                            FROM    pagosporsupervivencia, novedadporpersona
                            WHERE   pms_npp_codigo = $lqp_npp_codigo
                              AND   pms_npp_codigo = npp_codigo
                              AND   npp_registro_vigente AND
                              NOT   npp_anulada        AND
									novedadaterminadoflujo(npp_codigo)";

                    $consultaAux1->ejecutarConsulta($SQL);
                    list($pms_crl_codigo, $pms_ten_codigo, $pms_valor, $pms_plq_codigo_liquidado) = $consultaAux1->sigRegistroArreglo();

                    //Pagos por Mesada adicional
                    if (($pms_crl_codigo == 3 || $pms_crl_codigo == 17) && in_array($pms_plq_codigo_liquidado, $this->arreglo_plq_retroactivo))
                    {
                        if($this->_prt_valor == "" && $this->_prt_porcentaje != "")
                            $lqp_valor_criterio = ($lqp_valor_criterio) * (1 + ($this->_prt_porcentaje/100));
                        else
                            $lqp_valor_criterio = ($lqp_valor_criterio) + $this->_prt_valor;
                    }
                }
                * */

                unset($where_liquidacionnomina);
                unset($where_liquidacionretro);
                if ($lqp_npp_codigo != "")
                {
                    $where_liquidacionnomina = "AND lqd_npp_codigo = '$lqp_npp_codigo'";
                    $where_liquidacionretro = "AND lra_npp_codigo_liquidado = '$lqp_npp_codigo'";
                }

				//Dependiendo del criterio, se obtienen el valor y los días liquidados guardados en nómina
                switch($lqp_crl_codigo){
                  case '60':
                      //Si es un px1vez de criterios salariales que debieron 
                      //pagarse dentro del periodo retroactivo, aplica retroactividad.
                      $criteriosBaseSS=$this->_liquidacion->_seguridad_social->_criteriosBaseSeguridadSocialPension;
                      $SQL = "SELECT  puv_valor,lqd_dias_liquidados
                              FROM    pagosporunasolavez,liquidacionnomina
                              WHERE   puv_npp_codigo=$lqp_npp_codigo
                              AND     puv_registro_vigente
                              AND     puv_plq_codigo_origen in ($this->_cadena_arreglo_plq_cod)
                              AND     puv_crl_codigo in ($criteriosBaseSS)
                              AND     lqd_pel_codigo = '$this->_pel_codigo'
                              AND     lqd_plq_codigo = '$this->_plq_codigo'
                              AND     lqd_crl_codigo = '$lqp_crl_codigo'
                              AND     lqd_npp_codigo = puv_npp_codigo";
                              
                      $consultaAux1->ejecutarConsulta($SQL);
                      if($consultaAux1->numFilas()<=0){
                            continue;
                      }
                      
                  break;
                  case '6':
                  case '29':
                     $SQL = "SELECT  lqd_valor_criterio,lqd_dias_liquidados
                             FROM    liquidacionnomina
                             WHERE   lqd_registro_vigente
                                     AND lqd_pel_codigo = '$this->_pel_codigo'
                                     AND lqd_plq_codigo = '$this->_plq_codigo'
                                     AND lqd_crl_codigo = '$lqp_crl_codigo'
                                     $where_liquidacionnomina";
                     $consultaAux1->ejecutarConsulta($SQL);
                     if($consultaAux1->numFilas()<=0){
                       if($sql=$this->consultarPagoAdelantadoDelCriterio($lqp_crl_codigo)){
                           $consultaAux1->ejecutarConsulta($sql);
                       }
                     }  
                  break;
                
                  default:

                    $SQL = "SELECT  lqd_valor_criterio,lqd_dias_liquidados
                            FROM    liquidacionnomina
                            WHERE   lqd_registro_vigente
                                    AND lqd_pel_codigo = '$this->_pel_codigo'
                                    AND lqd_plq_codigo = '$this->_plq_codigo'
                                    AND lqd_crl_codigo = '$lqp_crl_codigo'
                                    $where_liquidacionnomina";
                    $consultaAux1->ejecutarConsulta($SQL);
                    if($consultaAux1->numFilas()<1 && $lqp_crl_codigo=='73'){
                      switch($this->_grl_codigo){
                          case '1':
                          case '2':
                          case '3':
                          case '5':
                          case '6':
                              $this->almacenarExcedenteSueldoBasicoParaPeriodoNoLiquidado(&$ArregloAImprimir);
                          break;
                      }
                    }
                }



				//Si encontró liquidaciones por cada criterio anteriormente
                if($consultaAux1->numFilas() > 0)
                {
                    while(list($lqd_valor_criterio,$lqd_dias_liquidados) = $consultaAux1->sigRegistroArreglo())
                    {
                       //Si el criterio es sueldo básico o sus afines, verifico si los días liquidados son
                       //menos de los correspondientes a los días de nómina del periodo, si es así, se verifica
                       //si esto se debe al solapamiento del disfrute de vacaciones pero para aquellas que fueron pagadas
                       //antes del periodo retroactivo. Si es así se debe pagar retro al periodo de liquidación completo.
                       switch($lqp_crl_codigo){
                            case '26':
                            case '72':
                            case '73':
                                if($lqd_dias_liquidados!=$this->_dias_nomina_del_periodo){
                                    if($this->diasSolapadosConDisfruteDeVacaciones()){
                                        $lqd_valor_criterio=($lqd_valor_criterio/$lqd_dias_liquidados)*$this->_dias_nomina_del_periodo;
                                    }
                                }
                            break;
                        }

                        $consultaAux2 = new Consulta($this->_bd);

						//Se obtiene el valor retroactivo a pagar aplicando la diferencia entre el valor de prenómina y el de nómina
                        $valor_retroactivo_a_pagar = $lqp_valor_criterio - $lqd_valor_criterio;
//echo "<p>retroactivo: $valor_retroactivo_a_pagar = prenomina: $lqp_valor_criterio - nomina : $lqd_valor_criterio</p>";
//echo "<p>dias laborados: $lqd_dias_liquidados</p>";
                        if ($this->_DEPURAR_RETROACTIVIDAD==1) {
							echo "<br>Criterio $lqp_crl_codigo";
							echo "<br>valor_retroactivo_a_pagar $valor_retroactivo_a_pagar = lqp_valor_criterio-lqd_valor_criterio => $lqp_valor_criterio-$lqd_valor_criterio ";
                        }
                        
                        
                        if ($lqd_valor_criterio > 0)
                        {
                        
                            switch($this->_prt_tipo){

                                case "NIVELACION":
                                    $crl_codigo=104;
                                break;

                                default;
                                    $crl_codigo=41;
                            }
                            //Busca si se ha realizado tareas de retroactividad anteriores y se restan a la actual
                            $SQL = "SELECT  sum(lra_valor_liquidado)
                                    FROM    liquidacionretroactividad,liquidacionnomina,periodoretroactivo
                                    WHERE   lra_pel_codigo = '$this->_pel_codigo'
                                            AND   lra_plq_codigo_liquidado = '$this->_plq_codigo'
                                            AND   lra_crl_codigo_liquidado = '$lqp_crl_codigo'
                                            AND   lra_registro_vigente $where_liquidacionretro
                                            AND   lra_prt_codigo=prt_codigo
                                            AND   prt_plq_codigo=lqd_plq_codigo
                                            AND   lra_pel_codigo=lqd_pel_codigo
                                            AND   lqd_crl_codigo in (41,104,111)
                                            AND   lqd_registro_vigente
                                            AND   lra_registro_vigente";
                            $valores_retroactivos_anteriores = $consultaAux2->obtenerCadenaResultadoConsulta($SQL);
                            $valor_retroactivo_a_pagar       = $valor_retroactivo_a_pagar - $valores_retroactivos_anteriores;

//if($valores_retroactivos_anteriores > 0){echo "<p>retroactivos anteriores: $valores_retroactivos_anteriores</p>";echo "<p>retro final: $valor_retroactivo_a_pagar</p>";}
                            if($this->_DEPURAR_RETROACTIVIDAD==1){
                              echo "<br>lqd_valor_criterio=$lqd_valor_criterio ";
                              echo "<br>valores_retroactivos_anteriores $valores_retroactivos_anteriores";
                              echo "<br>valor_retroactivo_a_pagar Despues $valor_retroactivo_a_pagar";
                            }

                            if ($valor_retroactivo_a_pagar > 1)//Es más costoso el trámite admitivo para pagar un peso.
                            {
                                if ($lqp_crl_codigo == "29" || $lqp_crl_codigo == "35" || $lqp_crl_codigo == "36" || $lqp_crl_codigo == "31"
                                    || $lqp_crl_codigo == "32" || $lqp_crl_codigo == "33" || $lqp_crl_codigo == "79")
                                {
                                    if ($this->_grl_codigo == "11" || $this->_grl_codigo == "12")
                                    {
                                        $SQL = "SELECT obtenerLiquidadoPorCriterioOtrosPeriodos($this->_pel_codigo,$this->_plq_codigo,$lqp_crl_codigo)";
                                        $pagos_otros_periodos = $consultaAux2->obtenerCadenaResultadoConsulta($SQL);
                                        $valor_retroactivo_a_pagar = $valor_retroactivo_a_pagar - $pagos_otros_periodos;
                                    }
                                }


                                if ($valor_retroactivo_a_pagar > 0)
                                {

                                    $DATOS_RETROACTIVOS["lra_pel_codigo"]            		= $this->_pel_codigo;
                                    $DATOS_RETROACTIVOS["lra_prt_codigo"]            		= $this->_prt_codigo;
                                    $DATOS_RETROACTIVOS["lra_valor_liquidado"]       		= round($valor_retroactivo_a_pagar);
                                    $DATOS_RETROACTIVOS["lra_crl_codigo_liquidado"]  		= $lqp_crl_codigo;
                                    $DATOS_RETROACTIVOS["lra_plq_codigo_liquidado"]  		= $this->_plq_codigo;
                                    $DATOS_RETROACTIVOS["lra_npp_codigo_liquidado"]  		= $lqp_npp_codigo;
                                    $DATOS_RETROACTIVOS["lra_dias_liquidados"]       		= $this->dias_liquidados;
                                    $DATOS_RETROACTIVOS["lra_npp_codigo_cambio_salario"]  	= $this->_arreglo_novedad[$this->_pel_codigo];

                                    //Se almacena en la Base de Datos la liquidaciòn del Retroactivo


                                    if(!$this->insertar($DATOS_RETROACTIVOS))
                                    {
                                        $this->_bd->abortarTransaccion();
                                        $error = new error("AVISO","AVISO4");
                                        $mesg = $error->retornarMensajeError();
                                        print "<center>$mesg</center>";
                                        $resp = FALSE;
                                    }
                                    else
                                    {
//echo "<br> <pre>";print_r($DATOS_RETROACTIVOS);
                                        if($lqp_crl_codigo!=20 && $lqp_crl_codigo!=21){
                                            if($this->_tipo=='PERSONA'){
                                              $SQL = "SELECT crl_nombre FROM criterioliquidacion WHERE  crl_codigo = '$lqp_crl_codigo'";
                                              $crl_nombre = $consultaAux2->obtenerCadenaResultadoConsulta($SQL);
                                              $valor = $ArregloAImprimir[$this->_pel_codigo]["Criterios"];
                                              $ArregloAImprimir[$this->_pel_codigo]["Criterios"][$crl_nombre] += $DATOS_RETROACTIVOS['lra_valor_liquidado'];
                                            }
                                        }

                                        if($lqp_crl_codigo==20 || $lqp_crl_codigo==21){
                                            $this->registrarIngresoBase($lqp_crl_codigo);
                                            if($this->_tipo=='PERSONA'){
                                              $SQL = "SELECT crl_nombre FROM criterioliquidacion WHERE  crl_codigo = '$lqp_crl_codigo'";
                                              $crl_nombre = $consultaAux2->obtenerCadenaResultadoConsulta($SQL);
                                              $ArregloAImprimir[$this->_pel_codigo]["Criterios"][$crl_nombre] += $DATOS_RETROACTIVOS['lra_valor_liquidado'];
                                            }
                                        }
                                    }
                                }//if ($valor_retroactivo_a_pagar > 0)
                                //duplicado carlosm TEMPORAL POR LA FALLA DE LA RETRO ....
                                else
                                {
                                    $valor = round($valor_retroactivo_a_pagar);
                                    $datos['lrn_crl_codigo']=$lqp_crl_codigo;
                                    $datos['lrn_valor_criterio']=$valor;                                
                                    $this->insertarValorNegativoOCero($datos);
                                }
                            }//if ($valor_retroactivo_a_pagar > 1
                            else
                            {
                                    $valor = round($valor_retroactivo_a_pagar);
                                    $datos['lrn_crl_codigo']=$lqp_crl_codigo;
                                    $datos['lrn_valor_criterio']=$valor;                                
                                    $this->insertarValorNegativoOCero($datos);                      
                            }
                        }//if ($lqd_valor_criterio > 0)
                       
                        $consultaAux2->liberarMemoria();
                    }//while
                }//if($consultaAux1->numFilas() > 0)
                $consultaAux1->liberarMemoria();

            }
        }
        if($this->_tipo!='PERSONA'){
          unset($ArregloAImprimirValoresCero);
          unset($ArregloAImprimirValoresNegativos);
          unset($ArregloAImprimir);
        }
        $consultaAux->liberarMemoria();
        unset($consultaAux1);
        unset($consultaAux);
        unset($consultaAux2);
        unset($DATOS_RETROACTIVOS);
        return $resp;
    }
    
    



    /**
     * Almacena la base de liquidación pensión o salud.
     *
     * @author      Alexander Varela Galvis
     * @access      public
     * @copyright   OITEL, Universidad del Valle 2.010
     * @since       2010-04-23
     * @return      void
     */
    function registrarIngresoBase($lqp_crl_codigo){

          switch($lqp_crl_codigo){
              case '20': //SEGURIDAD SOCIAL SALUD
                   $tabla   = "datosguardadosseguridadsocialsalud_pararetroactividad";
                   $prefijo = "dss_";
                   $crietio=113;
                   $tipo="Salud";
              break;
              case '21': //SEGURIDAD SOCIAL PENSION
                   $tabla   = "datosguardadosseguridadsocialpension_pararetroactividad";
                   $prefijo = "dsp_";
                   $crietio=114;
                   $tipo="Pensión";
              break;
          }

         $sql="SELECT   plq_fecha_inicio_periodo
                FROM    periodoliquidacion
                WHERE   plq_codigo=$this->_plq_codigo
                AND     plq_registro_vigente";
          $fecha_inicio_periodo=$this->_consulta->obtenerCadenaResultadoConsulta($sql);
          $datosFecha= explode("-",$fecha_inicio_periodo);
          $anho = $datosFecha[0];
          $mes  = $datosFecha[1]*1;

          $sql="SELECT  pel_per_codigo
                FROM    personaliquidacion
                WHERE   pel_codigo=$this->_pel_codigo
                AND     pel_registro_vigente";
          $per_codigo=$this->_consulta->obtenerCadenaResultadoConsulta($sql);

          $sql = "  SELECT  max(".$prefijo."ingreso_base_ajustado)
                    FROM    $tabla,personaliquidacion
                    WHERE   ".$prefijo."pel_codigo = pel_codigo
                        AND ".$prefijo."anho = $anho
                        AND ".$prefijo."mes = $mes
                        AND pel_per_codigo = $per_codigo";
          $ingresoBase=$this->_consulta->obtenerCadenaResultadoConsulta($sql);

          $DATOS_RETROACTIVOS["lra_pel_codigo"]            = $this->_pel_codigo;
          $DATOS_RETROACTIVOS["lra_prt_codigo"]            = $this->_prt_codigo;
          $DATOS_RETROACTIVOS["lra_valor_liquidado"]       = round($ingresoBase);
          $DATOS_RETROACTIVOS["lra_crl_codigo_liquidado"]  = $crietio;
          $DATOS_RETROACTIVOS["lra_plq_codigo_liquidado"]  = $this->_plq_codigo;
          $DATOS_RETROACTIVOS["lra_dias_liquidados"]       = $this->dias_liquidados;
          $DATOS_RETROACTIVOS["lra_npp_codigo_cambio_salario"]  = $this->_arreglo_novedad[$this->_pel_codigo];

          if(!$this->insertar($DATOS_RETROACTIVOS))
          {
              $error = new error("AVISO","AVISO165");
              $mesg = $error->retornarMensajeError();
              eval( "\$mesg = \"$mesg\";" );
              $error->imprimirMesajeEnJS ($mesg);
              print "<center>$mesg</center>";
              exit;
          }
     }


    /**
     * Inserta los datos en la tabla liquidacionretroactividad
     *
     * @author      Maricel Rubio
     * @access      public
     * @copyright   OIT, Universidad del Valle 2.004
     * @since       2004-11-29
     * @return      void
     */
    function insertar (&$DATOS_RETROACTIVOS)
    {
        $this->_tabla_liquidacion->establecerColumnas($DATOS_RETROACTIVOS);
        $this->_tabla_liquidacion->establecerSecuencias(array("lra_codigo"));
        $resp = $this->_tabla_liquidacion->insert($rest = "", $interfaz = TRUE, $cadena_insert = "", $insertar_a_la_fija=TRUE);

        return $resp;
    }// END function almacenarNomina



	/**
     * Establece los días laborados en el periodo de liquidacion a partir de la fecha de la novedad
     *
     * @author      Cristhian David Marín
     * @access      public
     * @copyright   OITEL, Universidad del Valle 2014
     * @since       2014
     * @return      boolean
     */

	function diasLaboradosPorNovedad (&$fecha_novedad, &$dias_restantes)
	{	
		$sql = "SELECT plq_fecha_inicio_periodo, plq_fecha_final_periodo
				FROM   periodoliquidacion
				WHERE  plq_codigo = $this->_plq_codigo
				AND	   plq_registro_vigente";

		$this->_consultaAux->ejecutarConsulta($sql);
		list($fecha_inicio_periodo_liquidacion, $fecha_fin_periodo_liquidacion) = $this->_consultaAux->sigRegistroArreglo();
		
		if (strtotime($fecha_novedad) < strtotime($fecha_inicio_periodo_liquidacion)) {

			$dias_restantes = 30;

			return true;

		} else if (strtotime($fecha_novedad) >= strtotime($fecha_inicio_periodo_liquidacion) &&
					strtotime($fecha_novedad) <= strtotime($fecha_fin_periodo_liquidacion)) {

			$dias_restantes = strtotime($fecha_fin_periodo_liquidacion) - strtotime($fecha_novedad) +1;
			$dias_restantes /= (60 * 60 * 24);

			return true;

		} else {

			return false;

		}
	}
	
	
	
	/**
     * Busca los puntos que tenia el docente antes del periodo retroactivo
     *
     * @author      Cristhian David Marín
     * @access      public
     * @copyright   OITEL, Universidad del Valle 2014
     * @since       2014
     * @return      array
     */

	function encontrarPuntosIniciales ()
	{
		$sql = "SELECT ndc_titulos_obtenidos, ndc_labor_docente, ndc_experiencia_profesional, ndc_experiencia_academica, ndc_produccion_intelectual, ndc_idiomas, ndc_distincion_academica
				FROM   nivelaciondocentecientoquince, novedadporpersona
				WHERE  npp_codigo = ndc_npp_codigo
				AND    npp_pel_codigo = $this->_pel_codigo
				AND    novedadaterminadoflujo(npp_codigo)
				AND    npp_fecha_novedad > '$this->_prt_fecha_inicio'
				AND    ndc_registro_vigente
				AND    npp_registro_vigente";

		$this->_consulta->ejecutarConsulta($sql);
		$puntos_novedades = array();
		$puntos_novedades[6] = 0;
		$puntos_novedades[7] = 0;
		$puntos_novedades[8] = 0;
		$puntos_novedades[9] = 0;
		$puntos_novedades[10] = 0;
		$puntos_novedades[11] = 0;
		$puntos_novedades[12] = 0;
		while (list($ndc_titulos_obtenidos, $ndc_labor_docente, $ndc_experiencia_profesional, $ndc_experiencia_academica, $ndc_produccion_intelectual, $ndc_idiomas, $ndc_distincion_academica) = $this->_consulta->sigRegistroArreglo()) {
			$puntos_novedades[6] = $puntos_novedades[6] + $ndc_titulos_obtenidos;
			$puntos_novedades[7] = $puntos_novedades[7] + $ndc_labor_docente;
			$puntos_novedades[8] = $puntos_novedades[8] + $ndc_experiencia_profesional;
			$puntos_novedades[9] = $puntos_novedades[9] + $ndc_experiencia_academica;
			$puntos_novedades[10] = $puntos_novedades[10] + $ndc_produccion_intelectual;
			$puntos_novedades[11] = $puntos_novedades[11] + $ndc_idiomas;
			$puntos_novedades[12] = $puntos_novedades[12] + $ndc_distincion_academica;
		}

		$sql = "SELECT pob_tpu_codigo, pob_cantidad_puntos
				FROM   puntosobtenidos
				WHERE  pob_per_codigo = (SELECT per_codigo FROM persona, personaliquidacion
										 WHERE	pel_codigo = $this->_pel_codigo
										 AND	pel_per_codigo = per_codigo
										 AND	per_registro_vigente
										 AND	pel_registro_vigente)--$this->_per_codigo
				AND    pob_registro_vigente";

		$this->_consulta->ejecutarConsulta($sql);
		while (list($tipo_codigo, $cantidad_puntos) = $this->_consulta->sigRegistroArreglo()) {
			//$puntos_novedades = $this->_consulta->obtenerCadenaResultadoConsulta($sql) - $puntos_novedades;
			$puntos_novedades[$tipo_codigo] = $cantidad_puntos - $puntos_novedades[$tipo_codigo];
		}

		return $puntos_novedades;
	}



	/**
	 * Calcula la formula de puntaje ponderado
	 * 
	 * @author      Cristhian David Marín
     * @access      public
     * @copyright   OITEL, Universidad del Valle 2014
     * @since       2014
     * @return      float
	 */
	function calcularPuntajePonderado (&$puntos_con_novedad)
	{
		$puntaje_ponderado = 0;
		foreach ($puntos_con_novedad as $tipo_punto => $cantidad_puntos) {
			$sql = "SELECT tpu_porcentaje FROM tipopunto
					WHERE  tpu_codigo = $tipo_punto
					AND	   tpu_registro_vigente";
			$ponderacion = $cantidad_puntos * $this->_consultaAux->obtenerCadenaResultadoConsulta($sql);

			$puntaje_ponderado = $puntaje_ponderado + $ponderacion;
		}

		return $puntaje_ponderado;
	}



	/**
     * Establece nuevo valor del SUELDO BASICO O CUOTA MENSUAL O AYUDA MENSUAL
     *
     * @author      Cristhian David Marín
     * @access      public
     * @copyright   OITEL, Universidad del Valle 2014
     * @since       2014
     * @return      void
     */

    function sueldoConRetroactivoPorNovedad (&$puntos_novedad)
    {
        $crl_codigo = '73';
		$this->_dias_nomina_del_periodo = 30;

        $this->_crl_codigo = $crl_codigo;

		//Consulta los retroactivos ya liquidados
        $SQL = "SELECT  sum(lra_valor_liquidado)
                FROM    liquidacionretroactividad,
                        liquidacionnomina,
                        periodoretroactivo
                WHERE   lra_pel_codigo = '$this->_pel_codigo'
                        AND   lra_plq_codigo_liquidado = '$this->_plq_codigo'
                        AND   lra_crl_codigo_liquidado = '$crl_codigo'
                        AND   lra_prt_codigo=prt_codigo
                        AND   prt_plq_codigo=lqd_plq_codigo
                        AND   lra_pel_codigo=lqd_pel_codigo
                        AND   lqd_crl_codigo in (41,104,111)
                        AND   lra_registro_vigente
                        AND   prt_registro_vigente
                        AND   lqd_registro_vigente";

        $retroactivo_liquidado = $this->_consulta->obtenerCadenaResultadoConsulta($SQL);

        $SQL = "SELECT  lqd_valor_criterio, lqd_dias_liquidados
                FROM    liquidacionnomina
                WHERE   lqd_pel_codigo='$this->_pel_codigo'
                        AND   lqd_plq_codigo = '$this->_plq_codigo'
                        AND   lqd_crl_codigo = '$crl_codigo'
                        AND   lqd_registro_vigente";

        $arreglo_tmp = explode(" ", $this->_consulta->obtenerCadenaResultadoConsulta($SQL));
        $sueldo_liquidado = $arreglo_tmp[0];

        $this->dias_liquidados = (!$arreglo_tmp[1])?0:$arreglo_tmp[1];

        switch ($this->_prt_tipo) {
			case 'NIVELACION':

				$puntos_con_novedad = array();
				$puntos_con_novedad[6] = $this->_puntos_base[6] + $puntos_novedad[6];
				$puntos_con_novedad[7] = $this->_puntos_base[7] + $puntos_novedad[7];
				$puntos_con_novedad[8] = $this->_puntos_base[8] + $puntos_novedad[8];
				$puntos_con_novedad[9] = $this->_puntos_base[9] + $puntos_novedad[9];
				$puntos_con_novedad[10] = $this->_puntos_base[10] + $puntos_novedad[10];
				$puntos_con_novedad[11] = $this->_puntos_base[11] + $puntos_novedad[11];
				$puntos_con_novedad[12] = $this->_puntos_base[12] + $puntos_novedad[12];

				//salario minimo por categoria
				$sql = "SELECT plq_fecha_inicio_periodo FROM periodoliquidacion
						WHERE plq_codigo = $this->_plq_codigo AND plq_registro_vigente";
				$fecha_plq = strtotime($this->_consulta->obtenerCadenaResultadoConsulta($sql));
				$sql = "SELECT smc_salario FROM salariominimocategoriadocentes115
						WHERE smc_cad_codigo = (SELECT cdp_cad_codigo FROM categoriadocentepersona
												WHERE cdp_per_codigo = $this->_per_codigo
												AND   cdp_registro_vigente)
						AND   smc_anno_vigencia = ".date("Y", $fecha_plq)."
						AND   smc_registro_vigente";
				$salario_minimo_categoria = $this->_consulta->obtenerCadenaResultadoConsulta($sql);

				//valor del punto
				$sql = "SELECT   vpd_valor_punto
						FROM     valorpuntodocente
						WHERE    vpd_grl_codigo = 1
						ORDER BY vpd_fecha_inicio DESC LIMIT 1";
				$valor_punto = $this->_consulta->obtenerCadenaResultadoConsulta($sql);

				//puntaje minimo por categoria
				$sql = "SELECT pmc_puntaje FROM puntajeminimocategoriadocentes115
						WHERE 	 pmc_cad_codigo = (SELECT cdp_cad_codigo FROM categoriadocentepersona
												   WHERE cdp_per_codigo = $this->_per_codigo
												   AND   cdp_registro_vigente)
						AND 	 pmc_registro_vigente
						ORDER BY pmc_fecha_inicio DESC LIMIT 1";
				$puntaje_minimo_categoria = $this->_consulta->obtenerCadenaResultadoConsulta($sql);

				//puntaje ponderado
				$puntaje_ponderado = $this->calcularPuntajePonderado($puntos_con_novedad);

				//dedicacion horaria
				$sql = "SELECT pca_cantidad_horas_x_semana FROM vinculacionplantacargo, cupoplantacargo, plantacargo
						WHERE    vpc_pel_codigo = $this->_pel_codigo
						AND		 vpc_cpc_codigo = cpc_codigo
						AND 	 cpc_pca_codigo = pca_codigo
						AND 	 cpc_registro_vigente
						AND 	 pca_registro_vigente
						ORDER BY vpc_codigo DESC limit 1";
				$dedicacion_horaria = $this->_consulta->obtenerCadenaResultadoConsulta($sql);

				$sueldo_retroactivo = $salario_minimo_categoria + $valor_punto * ($puntaje_ponderado - $puntaje_minimo_categoria);
				$sueldo_retroactivo = $sueldo_retroactivo * $dedicacion_horaria / 40;

				$dias_liquidados_en_periodo_actual = $this->dias_liquidados;
				//Si los días liquidados son diferentes a los días de nómina verificamos si el empleado estaba en vacaciones
				if ($dias_liquidados_en_periodo_actual!=$this->_dias_nomina_del_periodo) {
					if ($this->diasSolapadosConDisfruteDeVacaciones()) {
						$this->dias_liquidados=$dias_liquidados_en_periodo_actual=$this->_dias_nomina_del_periodo;
					}
				}

               $sueldo_retroactivo = (($sueldo_retroactivo / $this->_dias_nomina_del_periodo) * $dias_liquidados_en_periodo_actual);

          break;

          case 'VALOR':
              $sueldo_retroactivo = ($sueldo_liquidado + $retroactivo_liquidado) + $this->_prt_valor;
          break;
          case 'PORCENTAJE':
            $dias_liquidados_en_periodo_actual=$this->dias_liquidados;
            if($dias_liquidados_en_periodo_actual!=$this->_dias_nomina_del_periodo){
              if(($this->diasSolapadosConDisfruteDeVacaciones() + $dias_liquidados_en_periodo_actual)==$this->_dias_nomina_del_periodo){
                  $sueldo_liquidado=($sueldo_liquidado/$dias_liquidados_en_periodo_actual)*$this->_dias_nomina_del_periodo;
                  $dias_liquidados_en_periodo_actual=$this->_dias_nomina_del_periodo;
                  $sueldo_retroactivo=($sueldo_liquidado + $retroactivo_liquidado) * (1 + ($this->_prt_porcentaje/100));
              }
              if(isset($sueldo_retroactivo)){
                return $sueldo_retroactivo;            
              }
            }
            
            
            $sueldoNuevoProporcionalCalculado = ($sueldo_liquidado + $retroactivo_liquidado) * (1 + ($this->_prt_porcentaje/100));

            switch($this->_grl_codigo){
                  case '5':
                  case '6':
                  case '18':
                  case '19':
                      $this->_sueldoAnteriorEnUnMes=(!$this->_sueldoAnteriorEnUnMes)?$this->_sueldoNuevoEnUnMes:$this->_sueldoAnteriorEnUnMes;
                      $sueldoAnteriorProporcionalSegunNovedad = round(($this->_sueldoAnteriorEnUnMes/30)*$this->dias_liquidados);
                      $sueldoNuevoProporcionalSegunNovedad    = round(($this->_sueldoNuevoEnUnMes/30)*$this->dias_liquidados);
                      /*echo "<br><br> sueldoNuevoProporcionalSegunNovedad $sueldoNuevoProporcionalSegunNovedad";
                      echo "<br>this->dias_liquidados $this->dias_liquidados";
                      echo "<br>sueldoAnteriorProporcionalSegunNovedad $sueldoAnteriorProporcionalSegunNovedad";
                      echo "<br> sueldo_liquidado $sueldo_liquidado";
                      echo "<br> sueldo_liquidado $sueldo_liquidado".$this->_plq_codigo;*/
                      if($sueldo_liquidado==$sueldoAnteriorProporcionalSegunNovedad){
                        //Echo "<br>------>retorna sueldo_liquidado==sueldoNuevoProporcionalCalculado";
                        $sueldo_retroactivo=$sueldoNuevoProporcionalCalculado;
                      }elseif($sueldo_liquidado>=$sueldoNuevoProporcionalSegunNovedad){
                        //Echo "<br>------>retorna sueldo_liquidado>=sueldoNuevoProporcionalSegunNovedad";
                        $sueldo_retroactivo=$sueldo_liquidado;
                      }

                      if($sueldo_liquidado<$sueldoNuevoProporcionalSegunNovedad){
                        //Echo "<br>------>retorna sueldo_liquidado<$sueldoNuevoProporcionalSegunNovedad";
                        $sueldo_retroactivo=$sueldoNuevoProporcionalSegunNovedad;
                      }
                      
                  break;

                  case '1':
                        if(!$this->_sueldoNuevoEnUnMes){
                          $this->_sueldoNuevoEnUnMes = round(($sueldo_liquidado/$this->dias_liquidados)* 30 * (1 + ($this->_prt_porcentaje/100)));
                        }
                        $sueldo_retroactivo=$sueldoNuevoProporcionalCalculado;
                  break;

                  default:
                      $sueldo_retroactivo=$sueldoNuevoProporcionalCalculado;
            }//switch($this->_grl_codigo){

          break;

        }
        return $sueldo_retroactivo;
    }



    /**
     * Establece nuevo valor del SUELDO BASICO O CUOTA MENSUAL O AYUDA MENSUAL
     *
     * @author      Cristhian David Marín
     * @access      public
     * @copyright   OITEL, Universidad del Valle 2014
     * @since       2014
     * @return      void
     */

    function sueldoConRetroactivo ()
    {
        $crl_codigo = '73';
		$this->_dias_nomina_del_periodo = 30;

        $this->_crl_codigo = $crl_codigo;

		//Consulta los retroactivos ya liquidados
        $SQL = "SELECT  sum(lra_valor_liquidado)
                FROM    liquidacionretroactividad,
                        liquidacionnomina,
                        periodoretroactivo
                WHERE   lra_pel_codigo = '$this->_pel_codigo'
                        AND   lra_plq_codigo_liquidado = '$this->_plq_codigo'
                        AND   lra_crl_codigo_liquidado = '$crl_codigo'
                        AND   lra_prt_codigo=prt_codigo
                        AND   prt_plq_codigo=lqd_plq_codigo
                        AND   lra_pel_codigo=lqd_pel_codigo
                        AND   lqd_crl_codigo in (41,104,111)
                        AND   lra_registro_vigente
                        AND   prt_registro_vigente
                        AND   lqd_registro_vigente";

        $retroactivo_liquidado = $this->_consulta->obtenerCadenaResultadoConsulta($SQL);

        $SQL = "SELECT  lqd_valor_criterio, lqd_dias_liquidados
                FROM    liquidacionnomina
                WHERE   lqd_pel_codigo='$this->_pel_codigo'
                        AND   lqd_plq_codigo = '$this->_plq_codigo'
                        AND   lqd_crl_codigo = '$crl_codigo'
                        AND   lqd_registro_vigente";

        $arreglo_tmp = explode(" ", $this->_consulta->obtenerCadenaResultadoConsulta($SQL));
        $sueldo_liquidado = $arreglo_tmp[0];

        $this->dias_liquidados = (!$arreglo_tmp[1])?0:$arreglo_tmp[1];

        switch($this->_prt_tipo){
           case 'NIVELACION':
				//salario minimo por categoria
				$sql = "SELECT plq_fecha_inicio_periodo FROM periodoliquidacion
						WHERE plq_codigo = $this->_plq_codigo AND plq_registro_vigente";
				$fecha_plq = strtotime($this->_consulta->obtenerCadenaResultadoConsulta($sql));
				$sql = "SELECT smc_salario FROM salariominimocategoriadocentes115
						WHERE smc_cad_codigo = (SELECT cdp_cad_codigo FROM categoriadocentepersona
												WHERE cdp_per_codigo = $this->_per_codigo
												AND   cdp_registro_vigente)
						AND   smc_anno_vigencia = ".date("Y", $fecha_plq)."
						AND   smc_registro_vigente";
				$salario_minimo_categoria = $this->_consulta->obtenerCadenaResultadoConsulta($sql);

				//valor del punto
				$sql = "SELECT   vpd_valor_punto
						FROM     valorpuntodocente
						WHERE    vpd_grl_codigo = 1
						ORDER BY vpd_fecha_inicio DESC LIMIT 1";
				$valor_punto = $this->_consulta->obtenerCadenaResultadoConsulta($sql);

				//puntaje minimo por categoria
				$sql = "SELECT pmc_puntaje FROM puntajeminimocategoriadocentes115
						WHERE 	 pmc_cad_codigo = (SELECT cdp_cad_codigo FROM categoriadocentepersona
												   WHERE cdp_per_codigo = $this->_per_codigo
												   AND   cdp_registro_vigente)
						AND 	 pmc_registro_vigente
						ORDER BY pmc_fecha_inicio DESC LIMIT 1";
				$puntaje_minimo_categoria = $this->_consulta->obtenerCadenaResultadoConsulta($sql);

				//puntaje ponderado
				$puntaje_ponderado = $this->calcularPuntajePonderado($this->_puntos_base);

				//dedicacion horaria
				$sql = "SELECT pca_cantidad_horas_x_semana FROM vinculacionplantacargo, cupoplantacargo, plantacargo
						WHERE    vpc_pel_codigo = $this->_pel_codigo
						AND		 vpc_cpc_codigo = cpc_codigo
						AND 	 cpc_pca_codigo = pca_codigo
						AND 	 cpc_registro_vigente
						AND 	 pca_registro_vigente
						ORDER BY vpc_codigo DESC limit 1";
				$dedicacion_horaria = $this->_consulta->obtenerCadenaResultadoConsulta($sql);

				$sueldo_retroactivo = $salario_minimo_categoria + $valor_punto * ($puntaje_ponderado - $puntaje_minimo_categoria);
				$sueldo_retroactivo = $sueldo_retroactivo * $dedicacion_horaria / 40;

				$dias_liquidados_en_periodo_actual = $this->dias_liquidados;
				//Si los días liquidados son diferentes a los días de nómina verificamos si el empleado estaba en vacaciones
				if ($dias_liquidados_en_periodo_actual!=$this->_dias_nomina_del_periodo) {
					if ($this->diasSolapadosConDisfruteDeVacaciones()) {
						$this->dias_liquidados=$dias_liquidados_en_periodo_actual=$this->_dias_nomina_del_periodo;
					}
				}

				$sueldo_retroactivo = (($sueldo_retroactivo / $this->_dias_nomina_del_periodo) * $dias_liquidados_en_periodo_actual);

          break;

          case 'VALOR':
              $sueldo_retroactivo = ($sueldo_liquidado + $retroactivo_liquidado) + $this->_prt_valor;
          break;
          case 'PORCENTAJE':
            $dias_liquidados_en_periodo_actual=$this->dias_liquidados;
            if($dias_liquidados_en_periodo_actual!=$this->_dias_nomina_del_periodo){
              if(($this->diasSolapadosConDisfruteDeVacaciones() + $dias_liquidados_en_periodo_actual)==$this->_dias_nomina_del_periodo){
                  $sueldo_liquidado=($sueldo_liquidado/$dias_liquidados_en_periodo_actual)*$this->_dias_nomina_del_periodo;
                  $dias_liquidados_en_periodo_actual=$this->_dias_nomina_del_periodo;
                  $sueldo_retroactivo=($sueldo_liquidado + $retroactivo_liquidado) * (1 + ($this->_prt_porcentaje/100));
              }
              if(isset($sueldo_retroactivo)){
                return $sueldo_retroactivo;            
              }
            }
            
            
            $sueldoNuevoProporcionalCalculado = ($sueldo_liquidado + $retroactivo_liquidado) * (1 + ($this->_prt_porcentaje/100));

            switch($this->_grl_codigo){
                  case '5':
                  case '6':
                  case '18':
                  case '19':
                      $this->_sueldoAnteriorEnUnMes=(!$this->_sueldoAnteriorEnUnMes)?$this->_sueldoNuevoEnUnMes:$this->_sueldoAnteriorEnUnMes;
                      $sueldoAnteriorProporcionalSegunNovedad = round(($this->_sueldoAnteriorEnUnMes/30)*$this->dias_liquidados);
                      $sueldoNuevoProporcionalSegunNovedad    = round(($this->_sueldoNuevoEnUnMes/30)*$this->dias_liquidados);
                      /*echo "<br><br> sueldoNuevoProporcionalSegunNovedad $sueldoNuevoProporcionalSegunNovedad";
                      echo "<br>this->dias_liquidados $this->dias_liquidados";
                      echo "<br>sueldoAnteriorProporcionalSegunNovedad $sueldoAnteriorProporcionalSegunNovedad";
                      echo "<br> sueldo_liquidado $sueldo_liquidado";
                      echo "<br> sueldo_liquidado $sueldo_liquidado".$this->_plq_codigo;*/
                      if($sueldo_liquidado==$sueldoAnteriorProporcionalSegunNovedad){
                        //Echo "<br>------>retorna sueldo_liquidado==sueldoNuevoProporcionalCalculado";
                        $sueldo_retroactivo=$sueldoNuevoProporcionalCalculado;
                      }elseif($sueldo_liquidado>=$sueldoNuevoProporcionalSegunNovedad){
                        //Echo "<br>------>retorna sueldo_liquidado>=sueldoNuevoProporcionalSegunNovedad";
                        $sueldo_retroactivo=$sueldo_liquidado;
                      }

                      if($sueldo_liquidado<$sueldoNuevoProporcionalSegunNovedad){
                        //Echo "<br>------>retorna sueldo_liquidado<$sueldoNuevoProporcionalSegunNovedad";
                        $sueldo_retroactivo=$sueldoNuevoProporcionalSegunNovedad;
                      }
                      
                  break;

                  case '1':
                        if(!$this->_sueldoNuevoEnUnMes){
                          $this->_sueldoNuevoEnUnMes = round(($sueldo_liquidado/$this->dias_liquidados)* 30 * (1 + ($this->_prt_porcentaje/100)));
                        }
                        $sueldo_retroactivo=$sueldoNuevoProporcionalCalculado;
                  break;

                  default:
                      $sueldo_retroactivo=$sueldoNuevoProporcionalCalculado;
            }//switch($this->_grl_codigo){

          break;

        }
        return $sueldo_retroactivo;
    }

     /**
     * Si los días liquidados en el periodo son inferiores a 15 o 30 segun el tipo de periodo (quincenal o mensual)
     * se verifica si es porque el empleado estaba en vacaciones. Si es así, se le debe hacer retro por el número de días de nomina
     * del periodo, no sobre el número de días liquidados.
     * Solo se tienen en cuenta aquellas vacaciones que se solapan con el periodo retroactivo, pero que no fueron pagadas dentro de él.
     * @author      Alexander Varela Galvis
     * @access      public
     * @copyright   OIT, Universidad del Valle 2.009
     * @since       2009-10-06
     * @return      boolean
     */
    function diasSolapadosConDisfruteDeVacaciones(){
        $sql="SELECT  plq_fecha_inicio_periodo,plq_fecha_final_periodo
              FROM    periodoliquidacion
              WHERE   plq_codigo=$this->_plq_codigo
              AND     plq_registro_vigente";
        $this->_consulta->ejecutarConsulta($sql);
        list($plq_fecha_inicio_periodo,$plq_fecha_final_periodo)=$this->_consulta->sigRegistroArreglo();

        $sql="SELECT  diasdesolapamiento(npp_fecha_novedad,vac_fecha_finalizacion_novedad,date '$plq_fecha_inicio_periodo',date '$plq_fecha_final_periodo')
              FROM    liquidacionnomina,novedadporpersona,vacaciones
              WHERE   lqd_pel_codigo=$this->_pel_codigo
              AND     lqd_crl_codigo=14
              AND     lqd_npp_codigo=npp_codigo
              AND     npp_pel_codigo=lqd_pel_codigo
              AND     npp_codigo=vac_npp_codigo
              --AND     extract(year from npp_fecha_novedad)=extract(year from date '$plq_fecha_inicio_periodo')
              AND     diasdesolapamiento(npp_fecha_novedad,vac_fecha_finalizacion_novedad,date '$plq_fecha_inicio_periodo',date '$plq_fecha_final_periodo')>0
              AND     npp_fecha_novedad<'$this->_prt_fecha_inicio'
              AND NOT npp_anulada
              AND     vac_plq_codigo not in ($this->_cadena_arreglo_plq_cod)
              AND     novedadaterminadoflujo(npp_codigo)
              AND     lqd_registro_vigente
              AND     vac_registro_vigente";
        $numeroDias=$this->_consulta->obtenerCadenaresultadoconsulta($sql);

        if($numeroDias>0)
          return $numeroDias;
        else
          return 0;

    }



	/**
	 * Omite las nivelaciones que ya fueron liquidadas

     * @author      Cristhian David Marín
     * @access      public
     * @copyright   OITEL, Universidad del Valle 2014
     * @since       2014
     * @return      void
	 */
	function nivelacionesAOmitir ()
	{
		$sql = "SELECT DISTINCT lra_npp_codigo_liquidado FROM liquidacionretroactividad WHERE lra_registro_vigente";
		$this->_consulta->ejecutarConsulta($sql);

		$nivelaciones_npp_codigos = array();
		while (list($lra_npp_codigo_liquidado) = $this->_consulta->sigRegistroArreglo()) {

			$npp_codigos_liquidados = explode(",", $lra_npp_codigo_liquidado);
			foreach ($npp_codigos_liquidados as $codigo) {
				if ($codigo != "") {
					$sql = "SELECT ndc_npp_codigo FROM nivelaciondocentecientoquince
							WHERE  ndc_npp_codigo = $codigo AND ndc_registro_vigente";

					if ($this->_consultaAux->obtenerCadenaResultadoConsulta($sql) != "")
						array_push($nivelaciones_npp_codigos, $codigo);
				}
			}
		}

		$cadena_nivelaciones_a_omitir = "";
		if (count($nivelaciones_npp_codigos) > 0) {
			$sql = "SELECT DISTINCT lqd_npp_codigo FROM liquidacionnomina
					WHERE lqd_npp_codigo = ".implode(" OR lqd_npp_codigo = ", $nivelaciones_npp_codigos)." AND lqd_registro_vigente";

			$this->_consulta->ejecutarConsulta($sql);

			$arreglo_nivelaciones_liquidadas = array();
			while (list($lqd_npp_codigo) = $this->_consulta->sigRegistroArreglo()) {
				$arreglo_nivelaciones_liquidadas[] = $lqd_npp_codigo;
			}

			if (count($arreglo_nivelaciones_liquidadas) > 0) {
				$cadena_nivelaciones_a_omitir = implode(",", $arreglo_nivelaciones_liquidadas);
				$cadena_nivelaciones_a_omitir = "AND ndn_npp_codigo NOT IN (".$cadena_nivelaciones_a_omitir.")";
			}
		}

		return $cadena_nivelaciones_a_omitir;
	}



    /**
     * Establece en un arreglo las personas candidatas a aplicar la retroactividad

     * @author      Cristhian David Marín
     * @access      public
     * @copyright   OITEL, Universidad del Valle 2014
     * @since       2014
     * @return      void
     */
    function personasCandidatas ($grl_codigo)
    {
        $arreglo = implode (" OR lqd_plq_codigo = ", $this->arreglo_plq_retroactivo);

        switch($this->_prt_tipo){

          case "NIVELACION":

              $SQL = "
                  /*CREATE TEMPORARY TABLE PelCodigosAOmitir as (
                    SELECT npp_pel_codigo
                    FROM   novedadporpersona,nivelacion
                    WHERE  npp_codigo=niv_npp_codigo
                    AND    npp_fecha_novedad>='$this->_prt_fecha_inicio'
                    AND    niv_gra_codigo=1
                    AND    novedadaterminadoflujo(npp_codigo)
                    AND NOT npp_anulada);
                    CREATE INDEX PelCodigosAOmitir_idx ON PelCodigosAOmitir (npp_pel_codigo);*/

                  /*CREATE TEMPORARY TABLE NivelacionesAOmitir as (
                    SELECT distinct lra_npp_codigo_cambio_salario
                    FROM   liquidacionnomina,liquidacionretroactividad
                    WHERE  lqd_registro_vigente
                    AND    lra_pel_codigo=lqd_pel_codigo
                    AND    lra_npp_codigo_cambio_salario=lqd_npp_codigo);
                    CREATE INDEX CambiosDeSalarioAOmitir_idx ON CambiosDeSalarioAOmitir (lra_npp_codigo_cambio_salario);*/

                  CREATE TEMPORARY TABLE NivelacionesDocentesCientoQuince as (
                    SELECT DISTINCT ndc_npp_codigo
                    FROM   nivelaciondocentecientoquince,novedadporpersona
                    WHERE  ndc_npp_codigo=npp_codigo
                    --AND    npp_tnp_codigo=12
                    AND    novedadaterminadoflujo(npp_codigo)
                    AND NOT npp_anulada);

                  SELECT DISTINCT npp_pel_codigo/*,ndn_npp_codigo*/,pel_per_codigo
                    FROM    novedadporpersona,nivelaciondocentecientoquince,
                            personaliquidacion,liquidacionnomina
                    WHERE   novedadaterminadoflujo(npp_codigo)
                    AND     npp_codigo = ndc_npp_codigo
                    AND     ndc_npp_codigo IN (SELECT ndc_npp_codigo FROM NivelacionesDocentesCientoQuince)
                    AND NOT npp_anulada
                    AND     npp_fecha_novedad BETWEEN '$this->_prt_fecha_inicio' AND '$this->_prt_fecha_final'
                    AND     ndc_registro_vigente
                    AND     pel_registro_vigente
                    AND     npp_registro_vigente
                    AND     npp_pel_codigo = pel_codigo
                    AND     pel_grl_codigo = '$grl_codigo'
                    AND     pel_codigo = lqd_pel_codigo
                    AND     lqd_registro_vigente
                    AND     (lqd_plq_codigo = $arreglo)
                    --AND     npp_pel_codigo not in (select npp_pel_codigo from PelCodigosAOmitir)
                    ".$this->nivelacionesAOmitir();
          break;

          case "CAMBIOSALARIO":

              $SQL = "
                  CREATE TEMPORARY TABLE PelCodigosAOmitir as (
                    SELECT npp_pel_codigo
                    FROM   novedadporpersona,nivelacion
                    WHERE  npp_codigo=niv_npp_codigo
                    AND    npp_fecha_novedad>='$this->_prt_fecha_inicio'
                    AND    niv_gra_codigo=1
                    AND    novedadaterminadoflujo(npp_codigo)
                    AND NOT npp_anulada);
                    CREATE INDEX PelCodigosAOmitir_idx ON PelCodigosAOmitir (npp_pel_codigo);

                  CREATE TEMPORARY TABLE CambiosDeSalarioAOmitir as (
                    SELECT distinct lra_npp_codigo_cambio_salario
                    FROM   liquidacionnomina,liquidacionretroactividad
                    WHERE  lqd_registro_vigente
                    AND    lra_pel_codigo=lqd_pel_codigo
                    AND    lra_npp_codigo_cambio_salario=lqd_npp_codigo);
                    CREATE INDEX CambiosDeSalarioAOmitir_idx ON CambiosDeSalarioAOmitir (lra_npp_codigo_cambio_salario);

                  CREATE TEMPORARY TABLE CambiosDeSalarioCreadosPorCambioDeSalario as (
                    SELECT distinct cds_npp_codigo
                    FROM   cambiodesalario,novedadporpersona
                    WHERE  cds_npp_codigo_origen=npp_codigo
                    AND    npp_tnp_codigo=60
                    AND    novedadaterminadoflujo(npp_codigo)
                    AND NOT npp_anulada);

                  SELECT distinct npp_pel_codigo,cds_npp_codigo,pel_per_codigo
                    FROM    novedadporpersona,cambiodesalario,
                            personaliquidacion,liquidacionnomina
                    WHERE   novedadaterminadoflujo(npp_codigo)
                    AND     npp_codigo=cds_npp_codigo
                    AND     cds_npp_codigo in (SELECT cds_npp_codigo FROM CambiosDeSalarioCreadosPorCambioDeSalario)
                    AND NOT npp_anulada
                    AND     npp_fecha_novedad>='$this->_prt_fecha_inicio'
                    AND     npp_fecha_novedad<='$this->_prt_fecha_final'
                    AND     cds_registro_vigente
                    AND     pel_registro_vigente
                    AND     npp_registro_vigente
                    AND     npp_pel_codigo = pel_codigo
                    AND     pel_grl_codigo = '$grl_codigo'
                    AND     pel_codigo = lqd_pel_codigo
                    AND     lqd_registro_vigente
                    AND     (lqd_plq_codigo = $arreglo)
                    AND     npp_pel_codigo not in (select npp_pel_codigo from PelCodigosAOmitir)
                    AND     cds_npp_codigo not in (select lra_npp_codigo_cambio_salario from CambiosDeSalarioAOmitir);";
          break;

          case "PORCENTAJE":
              $SQL = "  CREATE TEMPORARY TABLE pelCodigosAOmitir as (
                            SELECT distinct lqd_pel_codigo
                            FROM   liquidacionnomina,liquidacionretroactividad,
                                   periodoretroactivo,personaliquidacion
                            WHERE  lra_prt_codigo=prt_codigo
                            AND    prt_fecha_inicio='$this->_prt_fecha_inicio'
                            AND    prt_porcentaje='$this->_prt_porcentaje'
                            AND    lra_pel_codigo=lqd_pel_codigo
                            AND    lqd_crl_codigo=41
                            AND    prt_plq_codigo=lqd_plq_codigo
                            AND    lra_pel_codigo=pel_codigo
                            AND    pel_grl_codigo='$this->_grl_codigo'
                            AND    lqd_registro_vigente
                            AND    lra_registro_vigente
                            AND    prt_registro_vigente
                            AND    pel_registro_vigente
                        );
                        CREATE INDEX pelCodigosAOmitir_idx ON pelCodigosAOmitir (lqd_pel_codigo);

                        SELECT  distinct pel_codigo,pel_per_codigo
                        FROM    personaliquidacion, liquidacionnomina
                        WHERE   pel_registro_vigente
                                AND pel_codigo = lqd_pel_codigo
                                AND pel_grl_codigo = '$grl_codigo'
                                AND lqd_registro_vigente
                                AND (lqd_plq_codigo = $arreglo)
                                AND lqd_pel_codigo not in (select lqd_pel_codigo from pelCodigosAOmitir)
                                --AND  pel_per_codigo in (9999)

                        ORDER BY pel_codigo";
           break;

          default:
              $SQL = "  SELECT  distinct pel_codigo,pel_per_codigo
                        FROM    personaliquidacion, liquidacionnomina
                        WHERE   pel_registro_vigente
                                AND pel_codigo = lqd_pel_codigo
                                AND pel_grl_codigo = '$grl_codigo'
                                AND lqd_registro_vigente
                                AND (lqd_plq_codigo = $arreglo)
                                --AND lqd_pel_codigo in (select pel_codigo from personaliquidacion,persona where pel_per_codigo=per_codigo and per_doc_ide_numero in (16692405,31239484,31860944,14965305,31869336,16581918,16585503,16239902,16581519,14984718,16621233))
                                --AND  pel_codigo=59765
                        ORDER BY pel_codigo
                ";
        }

        $this->_consultaPersonas->ejecutarConsulta($SQL);

    }



   /**
    * Este mètodo obtiene todos los periodos liquidados que estan dentro del periodo Retroactivo
    *
    * @author       Maricel Rubio
    * @access      public
    * @copyright   OIT, Universidad del Valle 2.003
    * @since       2004-11-05
    * @return
    */

    function obtenerPeridosYaLiquidados ()
    {

        if($this->_prt_tipo=='NIVELACION' || $this->_prt_tipo=='CAMBIOSALARIO'){
          $resto_where_tipo_periodo= ",2";
        }
        
         $SQL = " SELECT DISTINCT cgp_plq_codigo
                  FROM  criterioporgrupoporperiodoliquidacion,
                        criteriogrupoliquidacion,
                        periodoliquidacion
                  WHERE    cgp_cgl_codigo = cgl_codigo
                      AND  cgl_grl_codigo = '$this->_grl_codigo'
                      AND  cgp_plq_codigo=plq_codigo
                      AND  plq_fecha_inicio_periodo BETWEEN '$this->_prt_fecha_inicio' AND '$this->_prt_fecha_final'
                      AND plq_pcg_codigo IN (1,3 $resto_where_tipo_periodo)
                      AND  cgp_registro_vigente
                      AND  cgl_registro_vigente";

        $consultaAux = new Consulta($this->_bd);
        $consultaAux->ejecutarConsulta($SQL);

        if($consultaAux->numFilas()>0)
        {
             while(list($plq_codigo)=$consultaAux->sigRegistroArreglo())
             {
                 $arreglo_plq_cod[]=$plq_codigo;
             }
        }

        unset($consultaAux);
        return $arreglo_plq_cod;
     }


   /**
     * Obtiene todos los codigos de los periodo en los cuales la persona liquidacion esta vinculada
     *
     * @author      Maricel Rubio Susa
     * @access      public
     * @copyright   OIT, Universidad del Valle 2.004
     * @since       2004-09-03
     * @return      array
     */

    function obtenerCodigosPeriodoLiquidacion ($tipoArreglo="")
    {
        $arreglo_cods_plq = array();

        $liquidar_solo_criterios_adiciones = FALSE;

        $this->_liquidacion->_liquidar_solo_criterios_adiciones = $liquidar_solo_criterios_adiciones;

        $cadena_plq_retroactivos = implode(",",$this->arreglo_plq_retroactivo);

        $arreglo = explode (",", $cadena_plq_retroactivos);
        $cadena_plq_retroactivos = implode (" OR lqd_plq_codigo = ",$arreglo);
        $resto_where = "";

        $restoWhere=($tipoArreglo=="GLOBAL")?"":"AND lqd_pel_codigo = '$this->_pel_codigo'";

        if($tipoArreglo=="GLOBAL"){
            $SQL = "SELECT  DISTINCT lqd_plq_codigo,pcg_nombre
                    FROM    liquidacionnomina,
                            periodicidadcriteriogrupoliquidacion,
                            periodoliquidacion
                    WHERE   (lqd_plq_codigo = $cadena_plq_retroactivos)
                            AND   lqd_registro_vigente
                            AND   lqd_plq_codigo <> 0
                            AND   lqd_plq_codigo=plq_codigo
                            AND   plq_pcg_codigo=pcg_codigo
                            AND   plq_registro_vigente
                            --AND   plq_codigo in (699)
                            $restoWhere

                  ORDER BY  lqd_plq_codigo
             ";
        }else{
             $SQL = "SELECT  DISTINCT lqd_plq_codigo
                    FROM    liquidacionnomina
                    WHERE   (lqd_plq_codigo = $cadena_plq_retroactivos)
                            AND   lqd_registro_vigente
                            AND   lqd_plq_codigo <> 0
                            --AND   lqd_plq_codigo in (699)
                            $restoWhere

                  ORDER BY  lqd_plq_codigo
             ";
        }

        $consultaAux = new Consulta($this->_bd);
        $consultaAux->ejecutarConsulta($SQL);

        if($consultaAux->numFilas()>0)
        {
             while(list($plq_codigo,$pcg_nombre)=$consultaAux->sigRegistroArreglo())
             {
                 if($tipoArreglo=="GLOBAL"){
                      $arreglo_cods_plq[$plq_codigo] = $pcg_nombre;
                 }else{
                      $arreglo_cods_plq[] = $plq_codigo;
                 }
             }
        }
        unset($consultaAux);
        return $arreglo_cods_plq;
    }



    /**
     * Establece los parametros de la "claseLiquidacion.inc" para generar las tablas temporales de la nomina

     * @author      Maricel Rubio Susa
     * @access      public
     * @copyright   OIT, Universidad del Valle 2.004
     * @since       2004-09-03
     * @return      void
     */
    function establecerParametrosLiquidacion (&$HTTP_POST_VARS,$pel_codigo)
    {
        $HTTP_POST_VARS['tipo_nomina']       = 'PERSONA';
        $HTTP_POST_VARS['tipo_liquidacion']  = "liquidacionprenominatemporal";
        $HTTP_POST_VARS['tipo_periodicidad'] = "PERIODICO";
        $HTTP_POST_VARS['pel_codigo'] = $pel_codigo;

        $this->_liquidacion->establecerParametrosLiquidacion($HTTP_POST_VARS);

        $this->_manejar_transaccion = FALSE;
        $this->_liquidacion->_imprimirResultados    = FALSE;
        $this->_liquidacion->_imprimirResultados    = FALSE;
        $this->_liquidacion->_manejar_transaccion   = FALSE;
        $this->_liquidacion->_verificar_aprobacion  = TRUE;
        $this->_liquidacion->_comfirmar_liquidacion = FALSE;
        $this->_liquidacion->_verificar_procesos_ejecutados = FALSE;
        $this->liquidacionRetroactividad = TRUE;

        //$this->arreglo_plq_retroactivo = $this->obtenerPeridosYaLiquidados();

        if (count($this->arreglo_plq_retroactivo) > 0)
        {
            $this->_liquidacion->_periodos_a_liquidar = $this->arreglo_plq_retroactivo;
        }
        else
        {
            echo "<br><br><center>NO HAY PERIODOS A LIQUIDAR ENTRE LAS FECHAS DEL RETROACTIVO</center>";
            exit;
        }
        $this->_liquidacion->_fechaInicioLiquidacion = $this->_prt_fecha_inicio ;
        $this->_liquidacion->_fechaFinalizacionLiquidacion = $this->_prt_fecha_final ;
        $this->_liquidacion->generarTablasTemporales();
        $this->_liquidacion->_periodos_a_liquidar = array();
    }


    /**
     * Este metodo retorna un arreglo con los criterios de liquidaciòn que aplican a un grupo.
     * Estos criterios de liquidaciòn son a los que se les va a aplicar la retroactividad
     *
     *
     * @author      Maricel Rubio Susa
     * @access      public
     * param        grl_codigo  Còdigo del Grupo de Liquidaciòn
     * @copyright   OIT, Universidad del Valle 2.003
     * @since       2003-06-13
     * @return      array
     */
    function obtenerCriteriosLiquidacionGrupo ($plq_codigo)
    {
        $this->_liquidar_solo_criterios_adiciones = TRUE;

        if($this->_liquidar_solo_criterios_adiciones)
        {

            /*SEGURIDAD SOCIAL*/
            /*         21 | SEGURIDAD SOCIAL PENSION
                       78 | SEGURIDAD SOCIAL PENSION AJUSTE
                       20 | SEGURIDAD SOCIAL SALUD
                       77 | SEGURIDAD SOCIAL SALUD AJUSTE
                       24 | SOLIDARIDAD PENSIONAL
                       25 | SOLIDARIDAD PENSIONAL SUBSISTENC
                       37 | RETEFUENTE
            */

            /*  jubahamo: Se toman solo estas deducciones de ley pues las otras se calculan sobre el devengado
                y el devengado ya incluye el valor de la retroactividad, entonces sería una doble
                deducción.  Ej: Prodesarrollo, prounivalle.  La pregunta es, que pasa con un criterio
                que si requiera deduccion y no este en esta condición.
            */
            $complemento_resto_where="";
            IF($this->_prt_tipo!='NIVELACION' && $this->_prt_tipo!='CAMBIOSALARIO'){
              $complemento_resto_where = "
                      OR
                      (
                      (crl_tcl_nombre='DEDUCCION DE LEY' OR crl_tcl_nombre='SEGURIDAD SOCIAL' OR crl_tcl_nombre='OTRA DEDUCCION')
                      AND crl_codigo IN (21,59,78,20,77,24,25,37,89,113,114)
                      )";
            }

            /*
              5;"AUXILIO DEFUNCION"
              18;"SUBSIDIO TRANSPORTE"
              4;"AUXILIO MATERNIDAD"
              19;"SUBSIDIO FAMILIAR"
              43;"AUXILIO EDUCACION"
              36;"INTERESES CESANTIAS"
            */

            switch($this->_grl_codigo){
              case SUPERNUMERARIO;
              case CONTRATISTA_DOCENTE_HORA_CATEDRA;
              case CONTRATISTA_DOCENTE_OCASIONAL:
              case CONTRATISTA_A_TERMINO_FIJO:
                $complemento=" AND crl_codigo not in (19,43,4,5,18)";
              break;

              default:
                $complemento=" AND crl_codigo not in (36,19,18,43,5,4)"; //quité aux de educación = 43 --31843150 - ANA ARIAS
            }


            $resto_where="
                AND
                (

                    (
                        crl_tcl_nombre='ADICION'
                        $complemento
                    )
                    $complemento_resto_where
                )

            ";
        }

        $this->_liquidacion->_plq_codigo = $plq_codigo;
        $this->_liquidacion->construirFechaLiquidacion($arreglo=array());

        $where_quincena = $this->_liquidacion->obtenerWhereQuincena();

        $sql = "SELECT cgl_codigo, cgl_crl_codigo, crl_es_novedad, crl_nombre, crl_tcl_nombre,crl_orden
                FROM   criteriogrupoliquidacion, criterioliquidacion,criterioporgrupoporperiodoliquidacion $resto_from
                WHERE  cgl_crl_codigo = crl_codigo
                        AND cgl_codigo = cgp_cgl_codigo
                        AND cgl_grl_codigo = cgl_grl_codigo
                        AND cgl_grl_codigo = '$this->_grl_codigo'
                        AND cgp_plq_codigo = '$plq_codigo'
                        AND cgl_registro_vigente
                        AND crl_registro_vigente
                        AND cgp_registro_vigente
                       $resto_where
                       $where_quincena

                ORDER BY crl_orden,crl_nombre
            ";

        $this->_consulta->ejecutarConsulta($sql);

        if( $this->_consulta->numFilas() > 0){
            while (list($cgl_codigo, $cgl_crl_codigo, $crl_es_novedad, $crl_nombre, $tcl_nombre) = $this->_consulta->sigRegistroArreglo()){
                $datosCriterio["crl_es_novedad"] = $crl_es_novedad;
                $datosCriterio["crl_nombre"]  = $crl_nombre;
                $datosCriterio["tcl_nombre"]  = $tcl_nombre;
                $datosCriterio["cgl_crl_codigo"]  = $cgl_crl_codigo;

                $criteriosLiquidacion[$cgl_crl_codigo] = $datosCriterio;
            }
        }

        $this->_consulta->liberarMemoria();

        return $criteriosLiquidacion;

    }// END function obtenerCriteriosLiquidacionGrupo


    /**
     * Este mètodo obtiene los campos de la tabla retroactividad
     *
     * @author      Maricel Rubio
     * @access      public
     * @copyright   OIT, Universidad del Valle 2.003
     * @since       2004-11-05
     * @return
     */

     function tablaPeriodoRetroactivo ($HTTP_POST_VARS)
     {
        $prt_codigo=$HTTP_POST_VARS['prt_codigo'];

        $SQL="SELECT prt_fecha_inicio,prt_fecha_final,
                     prt_plq_codigo,prt_valor,
                     prt_porcentaje,prt_tipo
              FROM   periodoretroactivo
              WHERE  prt_registro_vigente
              AND    prt_codigo=$prt_codigo
         ";
        $consultaAux = new Consulta($this->_bd);
        $consultaAux->ejecutarConsulta($SQL);

        if($consultaAux->numFilas()>0)
        {
             while(list($prt_fecha_inicio,$prt_fecha_final,$prt_plq_codigo,$prt_valor,$prt_porcentaje,$prt_tipo)=$consultaAux->sigRegistroArreglo())
             {
                 $arreglo_retroactivo[0]=$prt_fecha_inicio;
                 $arreglo_retroactivo[1]=$prt_fecha_final;
                 $arreglo_retroactivo[2]=$prt_plq_codigo;
                 $arreglo_retroactivo[3]=$prt_valor;
                 $arreglo_retroactivo[4]=$prt_porcentaje;
                 $arreglo_retroactivo[5]=$prt_tipo;
             }
        }

        unset($consultaAux);
        return $arreglo_retroactivo;
    }


    /**
     * retorna un atributo de una tabla
     * @author    Maricel rubio
     * @access    public
     * @copyright OIT, Universidad del Valle 2.004
     * @since     2004-09-10
     * @return    void
     */
    function atributoTabla ($atributo,$tabla,$join,$registro_vigente)
    {
        $SQL = "SELECT  $atributo
                FROM    $tabla
                WHERE   $registro_vigente
                        AND $join
                 ";

        $consultaAux = new Consulta($this->_bd);
        $resultado   = $consultaAux->obtenerCadenaResultadoConsulta($SQL);
        unset($consultaAux);
        return $resultado;
    }


    /*
     * Imprime el cuadro HTMl con los valores calculados en la liquidación de retroactividad
     * @author    Alexander Varela Galvis
     * @access    public
     * @copyright OIT, Universidad del Valle 2.010     *      
     * @since     2010-11-22
     * @return    void
     */

    function imprimirResultados ($tipo_impresion)
    {
        if($this->_tipo=='PERSONA')
            $complemento="AND pel_codigo=$this->_pel_codigo";

        $where_retro_nivelacion="";
         if($this->_prt_tipo=='NIVELACION'){
              $where_retro_nivelacion = $this->obtenerWhereOmitirPelcodigos('EXCEDENTE POR NIVELACION','NIVELACION');
        }

        if($this->_prt_tipo=='CAMBIOSALARIO'){
              $where_retro_nivelacion = $this->obtenerWhereOmitirPelcodigos('RETROACTIVO','CAMBIOSALARIO');
        }
        
        if($this->_prt_tipo=='PORCENTAJE'){
              $where_retro_nivelacion = $this->obtenerWhereOmitirPelcodigos('RETROACTIVO','PORCENTAJE');
        }
        
        if($this->_formato=='Plano'){
              $nombre="ListadoRetroactividad-Periodo".$this->_prt_codigo;
              header('Content-Type: application/download');
              header("Content-Disposition: attachment; filename=$nombre.csv");
              echo "PEL_CODIGO|CEDULA|PRIMER_NOMBRE|SEGUNDO_NOMBRE|PRIMER_APELLIDO|SEGUNDO_APELLIDO|TIPO_CRITERIO|CRITERIO|VALOR_LIQUIDADO";
        }
        
        
        $sql = "SELECT  pel_codigo
                FROM    persona,personaliquidacion,liquidacionretroactividad
                WHERE   lra_pel_codigo=pel_codigo
                AND     pel_per_codigo=per_codigo
                AND     lra_prt_codigo=$this->_prt_codigo
                AND     pel_grl_codigo=$this->_grl_codigo
                $complemento
                $where_retro_nivelacion
                group by pel_codigo,per_primer_nombre
                order by per_primer_nombre,pel_codigo;";
        $this->_consultaAux2->ejecutarConsulta($sql);

        if($this->_consultaAux2->numFilas()==0){
            $tipo_impresion="No hay retroactividades a desplegar.";
        }

        if($this->_formato!='Plano'){
         echo "<span class=\"normalNegroB\">$tipo_impresion</span><br>
                <table width=\"98%\" border=\"0\" cellspacing=\"10\" cellpadding=\"0\" align=\"center\" valign=\"top\" bgcolor=\"#FFFFFF\">
                <tr align='right'>
                  <td colspan='2'>
                    <form action='index.php' method='post' target='_self' onSubmit=\"\">
                       <input type='hidden' value='ResumenRetro' name='accion'>
                       <input class='botonForma' type='submit' value='Volver al formulario' name='valor'>
                    </form>
                  </td>
                </tr>";                    
        }

        $i=1;
        $j=0;
        while(list($lqd_pel_codigo)=$this->_consultaAux2->sigRegistroArreglo())
        {
            $sql = "SELECT  per_doc_ide_numero,per_primer_nombre,per_segundo_nombre,
                            per_primer_apellido,per_segundo_apellido
                    FROM    persona,personaliquidacion
                    WHERE   pel_codigo=$lqd_pel_codigo
                    AND     pel_per_codigo=per_codigo
                    AND     per_registro_vigente";

            $this->_consultaAux->ejecutarConsulta($sql);
            $this->_pel_codigo=$lqd_pel_codigo;
            list($per_doc_ide_numero,$per_primer_nombre,$per_segundo_nombre,$per_primer_apellido,$per_segundo_apellido) = $this->_consultaAux->sigRegistroArreglo();

            $j++;
            if($this->_formato!='Plano'){
                if ($i % 2 != 0)
                {
                    echo "
                    <tr bgcolor=\"#FFFFFF\" align=\"center\" height=\"100%\">
                    ";
                }

                $PATH_PAQUETE_WEB=PATH_PAQUETE_WEB;
                $PATH_ICONOS        = PATH_ICONOS;
                echo "
                        <td width=\"50%\" valign=\"top\" align=\"top\" height=\"100%\">
                            <table cellspacing=\"1\" cellpadding=\"0\" width=\"100%\" valign=\"top\" bgcolor=\"#EAEAEA\" height=\"100%\">
                            <tr bgcolor=\"#EAEAEA\" height=\"20\">

                                <td class=\"normalAzul\">
                                    $j. $per_primer_nombre $per_segundo_nombre $per_primer_apellido $per_segundo_apellido
                                    <!--
                                    <form action='index.php' method='post' target='_self' onSubmit=\"\">
                                        <input type='hidden' name='accion' value='Borrar'>
                                        <input type='hidden' name='pel_codigo' value='$lqd_pel_codigo'>
                                        <input type='hidden' name='prt_codigo' value='$this->_prt_codigo'>
                                        $j - $per_primer_nombre $per_segundo_nombre $per_primer_apellido $per_segundo_apellido
                                        <input type='image' src='$PATH_ICONOS/ico_error.gif' name='boton' >
                                    </form>
                                    -->
                                </td>

                            </tr>

                            <tr><td>
                            <table border=\"0\" cellspacing=\"2\" cellpadding=\"0\" valign=\"top\" width=\"100%\" bgcolor=\"#FFFFFF\">
                                <tr>
                                    <td class=\"normalNegro\"> <b>Documento:</b> &nbsp;<span class=\"normalAzul\">$per_doc_ide_numero</span></td>
                                </tr>
                                <tr>
                                    <td class=\"normalNegro\"> <b>Vinculación:</b> &nbsp;<span class=\"normalAzul\">$lqd_pel_codigo</span></td>
                                </tr>

                                <tr>
                                    <td class=\"normalAzul\">
                                        <table width=\"100%\" border=\"0\" cellspacing=\"2\" cellpadding=\"0\" align=\"center\">
                ";
             }

            //IMPRIMIENDO VALORES A PAGAR
            $sql = "SELECT crl_tcl_nombre,crl_nombre,sum(lra_valor_liquidado)
                    FROM   liquidacionretroactividad,
                           criterioliquidacion
                    WHERE  lra_pel_codigo=$lqd_pel_codigo
                    AND    lra_prt_codigo=$this->_prt_codigo
                    AND    crl_codigo=lra_crl_codigo_liquidado
                    AND    crl_registro_vigente
                    AND    lra_crl_codigo_liquidado not in (113,114)
                    GROUP BY crl_tcl_nombre,crl_nombre,crl_orden ORDER BY crl_orden";



            $this->_consultaAux->ejecutarConsulta($sql);
            while (list($crl_tcl_nombre,$criterio,$lra_valor_liquidado) = $this->_consultaAux->sigRegistroArreglo()){
                $valor = number_format($lra_valor_liquidado);
                if($this->_formato!='Plano'){
                    echo "
                    <tr>
                        <td class=\"normalNegro\" width=\"95%\">
                            $criterio:
                        </td>

                        <td class = \"normalVinoTinto\" width=\"5%\" align=\"right\">
                            $valor
                        </td>
                    </tr>
                    ";
                }else{
                  $tipo=($crl_tcl_nombre=="ADICION")?"PAGO":"DESCUENTO";
                  echo "\n$lqd_pel_codigo|$per_doc_ide_numero|$per_primer_nombre|$per_segundo_nombre|$per_primer_apellido|$per_segundo_apellido|$tipo|$criterio|$lra_valor_liquidado";
                }
            }
            
            if($this->_formato=='Plano'){
                $sql="  SELECT crl_tcl_nombre,crl_nombre,lrn_valor_criterio
                        FROM   valoresretroactividadceroonegativos,criterioliquidacion,
                               periodoliquidacion
                        WHERE  lrn_pel_codigo=$lqd_pel_codigo
                        AND    lrn_prt_codigo=$this->_prt_codigo
                        AND    lrn_crl_codigo=crl_codigo
                        AND    lrn_plq_codigo=plq_codigo
                        AND    crl_registro_vigente
                        AND    plq_registro_vigente
                        AND    lrn_registro_vigente";
                $this->_consultaAux->ejecutarConsulta($sql);
                while (list($crl_tcl_nombre,$criterio,$lra_valor_liquidado) = $this->_consultaAux->sigRegistroArreglo()){
                    $valor = number_format($lra_valor_liquidado);
                      $tipo=($crl_tcl_nombre=="ADICION")?"PAGO":"DESCUENTO";
                      echo "\n$lqd_pel_codigo|$per_doc_ide_numero|$per_primer_nombre|$per_segundo_nombre|$per_primer_apellido|$per_segundo_apellido|$tipo|$criterio|$lra_valor_liquidado";
                }    
             }             
            
            //IMPRIMIENDO VALORES NEGATIVOS O CERO
            if($this->_formato!='Plano'){
            
            
                $sql = " SELECT crl_nombre ||' ('||lrn_plq_codigo||')',lrn_valor_criterio,
                                plq_fecha_inicio_periodo,plq_fecha_final_periodo
                         FROM   valoresretroactividadceroonegativos,criterioliquidacion,
                                periodoliquidacion
                         WHERE  lrn_pel_codigo=$lqd_pel_codigo
                         AND    lrn_prt_codigo=$this->_prt_codigo
                         AND    lrn_crl_codigo=crl_codigo
                         AND    lrn_plq_codigo=plq_codigo
                         AND    crl_registro_vigente
                         AND    plq_registro_vigente
                         AND    lrn_registro_vigente";
    
                $this->_consultaAux->ejecutarConsulta($sql);
                
                if($this->_consultaAux->numFilas()>0){
                
                      print "
                          <tr bgcolor=\"#EAEAEA\" height=\"20\">
                              <td class=\"normalAzul\" colspan='2'> VALORES CERO O NEGATIVOS</td>
                          </tr>";
                      
                      
                      while (list($criterio,$lra_valor_liquidado,$plq_fecha_inicio_periodo,$plq_fecha_final_periodo) = $this->_consultaAux->sigRegistroArreglo()){
                              $valor = number_format($lra_valor_liquidado);
                              unset($bgcolor);
                              if($valor<0){
                                $bgcolor="bgcolor='yellow'";                              
                              }
                              
                              echo "
                              <tr>
                                  <td class=\"normalNegro\" width=\"95%\" title='($plq_fecha_inicio_periodo - $plq_fecha_final_periodo)'>
                                      $criterio:
                                  </td>
          
                                  <td $bgcolor class = \"normalVinoTinto\" width=\"5%\" align=\"right\">
                                      $valor
                                  </td>
                              </tr>
                              ";
                      }
                 }//if($this->_consultaAux->numFilas()>0){     
            }//if($this->_formato!='Plano'){    

            if($this->_formato!='Plano'){
                echo "
                                        </table>
                                        </td></tr></table>
                                    </td>
                                </tr>
                            </table>
                        </td>
                ";


                if ($i % 2 == 0)
                {
                    echo "
                    </tr>
                    <tr height=\"15\"><td>&nbsp;</td></tr>
                    ";
                }
            }//if($this->_formato!='Plano')
            $i++;
        }

        if($this->_formato!='Plano'){
        
            echo "<tr align='right'>
                  <td colspan='2'>
                    <form action='index.php' method='post' target='_self' onSubmit=\"\">
                       <input type='hidden' value='ResumenRetro' name='accion'>
                       <input class='botonForma' type='submit' value='Volver al formulario' name='valor'>
                    </form>
                  </td>
                </tr>
                </table><br><br>";
        }
    }


    /**
     * Crea la barra de progreso.
     *
     * @author      Alexander Varela Galvis <alvarela@univalle.edu.co>
     * @access      public
     * @copyright   OITEL, Universidad del Valle 2.007
     * @since       2007-03-26
     * @return      object
     */
    function CrearBarra ($personasLiquidacion)
    {
       if ($personasLiquidacion > 0 )
        {

            // Inicio parametros de la clase class.progressbar.inc para la generación de una barra de progreso
            $prb = new ProgressBar(300, 20);    // create new ProgressBar (width:300px,height:30px)
            $prb->max = $personasLiquidacion;
            $prb->left = 300;   // posicion desde la izquierda
            $prb->top = 180;    // posicion dese arriba
            $prb->border = 0;
            $prb->color = '#FB7D7E';
            $prb->bgr_color = '#EAE5EB';
            $prb->addLabel('percent','pct1');
            $prb->setLabelPosition('pct1',300,185,300,100,'center');
            $prb->setLabelFont('pct1',14);
            $prb->addLabel('text','txt1');
            $prb->setLabelPosition('txt1',300,185,100,100,'center');
            $prb->setLabelFont('txt1',14);
            $prb->setLabelValue('txt1','Liquidando ...');
            $prb->show();   // show the ProgressBar
        }
        return $prb;
    }

    /**
     * Muestra el formulario para seleccionar las opciones para imprimir el resumen de retroactividades generadas.
     *
     * @author      Alexander Varela Galvis <alvarela@univalle.edu.co>
     * @access      public
     * @copyright   OITEL, Universidad del Valle 2.009
     * @since       2009-03-19
     * @return      object
     */
    function mostrarFormularioResumenes ($personasLiquidacion)
    {
        include ("fm_RetroactividadResumen.php");
    }


    /**
     * Inicializa las variables necesarias de acuerdo a la selección del usuario en el
     * formulario fm_RetroactividadResumen
     *
     * @author      Alexander Varela Galvis <alvarela@univalle.edu.co>
     * @access      public
     * @copyright   OITEL, Universidad del Valle 2.009
     * @since       2009-03-19
     * @return      object
     */
    function inicializarDatosResumen($HTTP_POST_VARS)
    {
        $arreglo_datos_retroactividad = $this->tablaPeriodoRetroactivo($HTTP_POST_VARS);
        $this->_prt_fecha_inicio =   $arreglo_datos_retroactividad[0];
        $this->_prt_fecha_final  =   $arreglo_datos_retroactividad[1];
        $this->_plq_codigo_pago  =   $arreglo_datos_retroactividad[2];
        $this->_prt_valor        =   $arreglo_datos_retroactividad[3];
        $this->_prt_porcentaje   =   $arreglo_datos_retroactividad[4];
        $this->_prt_tipo         =   $arreglo_datos_retroactividad[5];
        $this->_liquidacion->_fechaInicioLiquidacion = $this->_prt_fecha_inicio ;
        $this->_liquidacion->_fechaFinalizacionLiquidacion = $this->_prt_fecha_final ;

        $this->_prt_codigo       =   $HTTP_POST_VARS['prt_codigo'];


        $this->_tipo=$HTTP_POST_VARS['tipo_aplicacion_retroactivo'];
        $this->_origen_datos=$HTTP_POST_VARS['origen_datos'];
        $this->_formato     =$HTTP_POST_VARS['formato'];


        if($this->_tipo=='PERSONA'){
            $this->_grl_codigo   =  $HTTP_POST_VARS['pel_grl_codigo'];
            $this->_pel_codigo   =  $HTTP_POST_VARS['pel_codigo'];
        }else {
          $this->_grl_codigo       =   $HTTP_POST_VARS['grl_codigo'];
        }
        $this->arreglo_plq_retroactivo = $this->obtenerPeridosYaLiquidados();
        $this->imprimirResultados("Retroactivos Generados:");
    }
    
    
     /**
     * Mueve los valores liquidados por retroactividad de un periodo retroactivo
     * a otro, sólo para las personas a las cuales no se les pagó dicha retroactividad
     * por algún motivo.          
     *
     * @author      Alexander Varela Galvis <alvarela@univalle.edu.co>
     * @access      public
     * @copyright   OITEL, Universidad del Valle 2.011
     * @since       2012-05-09
     * @return      object
     */
    function mostrarFormulario_moverRetroactividad ($HTTP_POST_VARS)
    {
      include_once("fm_MoverRetroactividad.php");       
    }
    
    /**
     * Mueve los valores liquidados por retroactividad de un periodo retroactivo
     * a otro, sólo para las personas a las cuales no se les pagó dicha retroactividad
     * por algún motivo.          
     *
     * @author      Alexander Varela Galvis <alvarela@univalle.edu.co>
     * @access      public
     * @copyright   OITEL, Universidad del Valle 2.011
     * @since       2012-05-09
     * @return      object
     */
    function moverRetroactividadANuevoPeriodo ($HTTP_POST_VARS)
    {
      $prt_origen=$HTTP_POST_VARS['prt_codigo_inactivo'];
      $prt_destino=$HTTP_POST_VARS['prt_codigo_activo'];
      $grl_codigo=$HTTP_POST_VARS['grl_codigo'];
      
      $sql=" SELECT prt_plq_codigo,prt_tipo
             FROM   periodoretroactivo
             WHERE  prt_codigo=$prt_origen
             AND    prt_registro_vigente";
      $this->_consulta->ejecutarConsulta($sql);
      list($plq_de_pago,$tipo)=$this->_consulta->sigRegistroArreglo();
      
      switch($tipo){
        case 'NIVELACION':
          $crl_codigo=104;
        break;  
        case 'PORCENTAJE':
          $crl_codigo=41;
        break;
        case 'CAMBIOSALARIO':
          $crl_codigo=111;
        break;
      }
      
      $this->_bd->iniciarTransaccion();
      
      $sql="CREATE TEMPORARY TABLE retrosnopagadas AS(
                SELECT *
                FROM   liquidacionretroactividad,personaliquidacion
                WHERE  lra_prt_codigo=$prt_origen
                AND    lra_pel_codigo=pel_codigo
                AND    pel_grl_codigo=$grl_codigo
                AND    lra_pel_codigo NOT IN (SELECT distinct lqd_pel_codigo
                                              FROM   liquidacionnomina,personaliquidacion
                                              WHERE  lqd_plq_codigo=$plq_de_pago
                                              AND    lqd_pel_codigo=pel_codigo
                                              AND    lqd_crl_codigo=$crl_codigo
                                              AND    pel_grl_codigo=$grl_codigo
                                              AND    pel_registro_vigente
                                              AND    lqd_registro_vigente )
                                      )";
      $this->_consulta->ejecutarConsulta($sql);
      
      $where="lra_prt_codigo=$prt_destino
              AND   lra_pel_codigo=retrosnopagadas.lra_pel_codigo";               
      $this->_tabla_liquidacion->deleteReal($where,$rest = "", $interfaz = TRUE, $cadena_delete = "", $delete_a_la_fija=TRUE);
      
      $sql = "SELECT count(*)
              FROM   liquidacionretroactividad
              WHERE  lra_pel_codigo=retrosnopagadas.lra_pel_codigo
              AND    lra_prt_codigo='$prt_origen'";
      $cantidad=$this->_consulta->obtenerCadenaResultadoConsulta($sql);
      if($cantidad>0){
          $cadena_update = "UPDATE liquidacionretroactividad
                            SET    lra_prt_codigo='$prt_destino'
                            WHERE  lra_pel_codigo=retrosnopagadas.lra_pel_codigo
                            AND    lra_prt_codigo='$prt_origen'";
          if($this->_tabla_liquidacion->update($cadenaWhere = "", $rest = "",$interfaz = FALSE, $cadena_update,$mostrarMensaje=false)){
            $error = new error("AVISO","AVISO177");
            $mesg = $error->retornarMensajeError();
            eval( "\$mesg = \"$mesg\";" );
            print "<center>$mesg</center><br><br>";
            //$this->_bd->abortarTransaccion();
            $this->_bd->confirmarTransaccion();
          }else{
            $this->_bd->abortarTransaccion();
          }
      }else{
            $error = new error("AVISO","AVISO178");
            $mesg = $error->retornarMensajeError();
            eval( "\$mesg = \"$mesg\";" );
            print "<center>$mesg</center><br><br>";
            $this->_bd->abortarTransaccion();
      }
    }



}//end class

?>
