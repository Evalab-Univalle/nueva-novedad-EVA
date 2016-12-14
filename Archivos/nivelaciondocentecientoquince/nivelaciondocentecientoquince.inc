<?php

    include_once ("C:\AppServ/www/srh2/public_html/configuracion/config.php");    
$PATH_SERVIDOR_REAL = PATH_SERVIDOR_REAL;
    include_once ("$PATH_SERVIDOR_REAL/configuracion/config.php");
    include_once PATH_PAQUETE."/nomina/novedad/novedadporpersona/vinculacionlaboral/vinculacionoficial/vinculacionoficial.inc";
    include_once PATH_PAQUETE."/nomina/novedad/novedadporpersona/vinculacionlaboral/vinculacionpublico/vinculacionpublico.inc";
    include_once PATH_PAQUETE."/nomina/novedad/novedadporpersona/vinculacionlaboral/vinculacionsupernumerario/vinculacionsupernumerario.inc";
    include_once PATH_PAQUETE."/nomina/novedad/novedadporpersona/vinculacionlaboral/vinculacioncontratistaterminofijo/vinculacioncontratistaterminofijo.inc";


/**
 * Clase nivelaciondocentecientoquince
 * Clase para manejar el objeto nivelacion del decreto 1279
 *
 *
 * @access  public
 * @package TablaMaestro
 * @subpackage nivelaciondocentecientoquince
 * @copyright   OIT, Universidad del Valle 2.014
 * @author  Cristhian David Marín <cristhian.marin@correounivalle.edu.co>
 * @version 1.0
 *
 */



class nivelaciondocentecientoquince extends tablaMaestroNovedad{

    /**
    * Objeto de tipo TABLA, para hacer referencia al objeto: _objeto_maestro
    * @var object
    */
    var $_nivelaciondocentecientoquince = "";

    var $_consulta_aux;


    /**
     * Constructor de la clase nivelaciondocentecientoquince
     *
     * @access  public
     * @copyright   OITEL, Universidad del Valle 2.014
     * @author  Cristhian David Marín <cristhian.marin@correounivalle.edu.co>
     * @param   string    $aplicacion   nombre de la aplicacion
     * @param   string    $usu_login    nombre de usuario
     * @param   string    $grupo        grupo del usuario
     * @param   object    &$bd          Objeto La base de datos, paso por referencia
     * @return  void
     *
     */
    function nivelaciondocentecientoquince ($aplicacion, $usu_login, $grupo, &$bd, $maestro_independiente=FALSE)
    {
        $this->_nombre_tabla="nivelaciondocentecientoquince";
        $this->_nombre_llave_padre = "npp_codigo";
        if(!$maestro_independiente)
        {
            parent::tablaMaestroNovedad($aplicacion,$usu_login, $grupo,  $bd);
            $this->_nivelacion = &$this->_objeto_maestro;
            $datosValidacion = array("ndc_titulos_obtenidos"=>"num|TITULOS OBTENIDOS|min|max|no_null",
									 "ndc_labor_docente"=>"num|LABOR DOCENTE|min|max|no_null",
									 "ndc_experiencia_profesional"=>"num|EXPERIENCIA PROFESIONAL|min|max|no_null",
									 "ndc_experiencia_academica"=>"num|EXPERIENCIA ACADÉMICA|min|max|no_null",
									 "ndc_produccion_intelectual"=>"num|PRODUCCIÓN INTELECTUAL|min|max|no_null",
									 "ndc_idiomas"=>"num|IDIOMAS|min|max|no_null",
									 "ndc_distincion_academica"=>"num|DISTINCIÓN ACADÉMICA|min|max|no_null"
                                     );
            $this->_objeto_maestro->establecerDatosValidacion($datosValidacion);
			$this->_consulta_aux = new CONSULTA($bd);
        }
        else
        {
            $this->_usu_login=$usu_login;
            parent::FormaPlantilla($aplicacion,  $grupo,  $bd);
            $this->_objeto_maestro = new TABLA($this->_nombre_tabla, $this->_usu_login,
                                               $this->_grupo, $this->_bd, $tabla_independiente = TRUE);
            $this->_objeto_maestro->_atributosLlaves=array("ndc_npp_codigo");
            $this->_objeto_maestro->_pre="ndc_";
        }
        
        $this->_clasenovedadporpersona = new Novedadporpersona($aplicacion, $usu_login, $grupo, $bd, $maestro_independiente);

    }



    /**
     * Verifica que la fecha del traslado sea mayor que el ultimo movimiento registrado
     *
     * @author    Cristhian David Marín <cristhian.marin@correounivalle.edu.co>
     * @access    public
     * @copyright OITEL, Universidad del Valle 2014
     * @return    boolean
     */

    function validarFechaInicioFin (&$HTTP_POST_VARS)
    {
        $vpc_fecha_inicio_nueva = $HTTP_POST_VARS["npp_fecha_novedad"];
        $pel_codigo = $HTTP_POST_VARS["npp_pel_codigo"];

        //alvarela:anulacion:2006-AGT-04, adiciono AND npp_registro_vigente AND NOT npp_anulada
        $SQL = "SELECT   npp_fecha_novedad
                FROM     novedadporpersona, nivelaciondocentecientoquince
                WHERE    npp_codigo = ndc_npp_codigo
                         AND npp_pel_codigo = '$pel_codigo'
                         AND npp_registro_vigente
                         AND NOT npp_anulada
                         AND novedadATerminadoFlujo(npp_codigo)
                ORDER BY npp_fecha_novedad desc";

        $this->_consulta->ejecutarConsulta($SQL);

        if($this->_consulta->numFilas() > 0)
            $vpc_fecha_inicio_antigua = $this->_consulta->obtenerCadenaResultadoConsulta($SQL);
        else
            $vpc_fecha_inicio_antigua = $HTTP_POST_VARS["vpc_fecha_inicio"];

        if($vpc_fecha_inicio_antigua <= $vpc_fecha_inicio_nueva)
        {
               return TRUE;
        }
        else
        {
            $error = new error("ERROR","ERROR604");
            $mesg = $error->retornarMensajeError();
            print "<center>$mesg <span class=\"error\"></span></center>";
            return FALSE;
        }

        return TRUE;
    }




    /**
     * Este Método es el encargado de realizar el Moviento de la Persona en la Planta de Cargos
     *
     * @author      Cristhian David Marín <cristhian.marin@correounivalle.edu.co>
     * @access      public
     * @copyright   OITEL, Universidad del Valle 2014
     * @return      void
     */

    function realizarOperacionesDeAprobacionNovedad (&$HTTP_POST_VARS)
    {
		$respuesta = FALSE;
		$this->_puntosobtenidos = new TABLA("puntosobtenidos", $this->_usu_login, $this->_grupo, $this->_bd);

        $npp_codigo = $HTTP_POST_VARS["npp_codigo"];
        $pel_codigo = $HTTP_POST_VARS["npp_pel_codigo"];

		$SQL = "SELECT pel_per_codigo 
		        FROM   personaliquidacion 
		        WHERE  pel_codigo = $pel_codigo
		        AND    pel_registro_vigente";
		        
        $per_codigo = $this->_consulta->obtenerCadenaResultadoConsulta($SQL); //Obtengo la persona

        //Ahora dependiendo de que campo se añadieron puntos, actualizo la totalidad de puntos del afectado
        if ($HTTP_POST_VARS['ndc_titulos_obtenidos'] != 0) {
			$ndc_titulos_obtenidos = $HTTP_POST_VARS['ndc_titulos_obtenidos'];
			$cadena_update = "UPDATE puntosobtenidos
							  SET    pob_cantidad_puntos = COALESCE(pob_cantidad_puntos, 0) + $ndc_titulos_obtenidos
							  WHERE  pob_per_codigo = $per_codigo
							  AND    pob_tpu_codigo = 6
							  AND    pob_registro_vigente";
					
            $this->_puntosobtenidos->update($cadenaWhere = "", $rest = "", $interfaz = FALSE, $cadena_update, $mostrarMensaje = FALSE);
		}
		
		if ($HTTP_POST_VARS['ndc_labor_docente'] != 0) {
			$ndc_labor_docente = $HTTP_POST_VARS['ndc_labor_docente'];
			$cadena_update = "UPDATE puntosobtenidos
							  SET    pob_cantidad_puntos = COALESCE(pob_cantidad_puntos, 0) + $ndc_labor_docente
							  WHERE  pob_per_codigo = $per_codigo
							  AND    pob_tpu_codigo = 7
							  AND    pob_registro_vigente";
					
            $this->_puntosobtenidos->update($cadenaWhere = "", $rest = "", $interfaz = FALSE, $cadena_update, $mostrarMensaje = FALSE);
		}
		
		if ($HTTP_POST_VARS['ndc_experiencia_profesional'] != 0) {
			$ndc_experiencia_profesional = $HTTP_POST_VARS['ndc_experiencia_profesional'];
			$cadena_update = "UPDATE puntosobtenidos
							  SET    pob_cantidad_puntos = COALESCE(pob_cantidad_puntos, 0) + $ndc_experiencia_profesional
							  WHERE  pob_per_codigo = $per_codigo
							  AND    pob_tpu_codigo = 8
							  AND    pob_registro_vigente";
					
            $this->_puntosobtenidos->update($cadenaWhere = "", $rest = "", $interfaz = FALSE, $cadena_update, $mostrarMensaje = FALSE);
		}
		
		if ($HTTP_POST_VARS['ndc_experiencia_academica'] != 0) {
			$ndc_experiencia_academica = $HTTP_POST_VARS['ndc_experiencia_academica'];
			$cadena_update = "UPDATE puntosobtenidos
							  SET    pob_cantidad_puntos = COALESCE(pob_cantidad_puntos, 0) + $ndc_experiencia_academica
							  WHERE  pob_per_codigo = $per_codigo
							  AND    pob_tpu_codigo = 9
							  AND    pob_registro_vigente";

            $this->_puntosobtenidos->update($cadenaWhere = "", $rest = "", $interfaz = FALSE, $cadena_update, $mostrarMensaje = FALSE);
		}
		
		if ($HTTP_POST_VARS['ndc_produccion_intelectual'] != 0) {
			$ndc_produccion_intelectual = $HTTP_POST_VARS['ndc_produccion_intelectual'];
			$cadena_update = "UPDATE puntosobtenidos
							  SET    pob_cantidad_puntos = COALESCE(pob_cantidad_puntos, 0) + $ndc_produccion_intelectual
							  WHERE  pob_per_codigo = $per_codigo
							  AND    pob_tpu_codigo = 10
							  AND    pob_registro_vigente";
					
            $this->_puntosobtenidos->update($cadenaWhere = "", $rest = "", $interfaz = FALSE, $cadena_update, $mostrarMensaje = FALSE);
		}
		
		if ($HTTP_POST_VARS['ndc_idiomas'] != 0) {
			$ndc_idiomas = $HTTP_POST_VARS['ndc_idiomas'];
			$cadena_update = "UPDATE puntosobtenidos
							  SET    pob_cantidad_puntos = COALESCE(pob_cantidad_puntos, 0) + $ndc_idiomas
							  WHERE  pob_per_codigo = $per_codigo
							  AND    pob_tpu_codigo = 11
							  AND    pob_registro_vigente";
					
            $this->_puntosobtenidos->update($cadenaWhere = "", $rest = "", $interfaz = FALSE, $cadena_update, $mostrarMensaje = FALSE);
		}
		
		if ($HTTP_POST_VARS['ndc_distincion_academica'] != 0) {
			$ndc_distincion_academica = $HTTP_POST_VARS['ndc_distincion_academica'];
			$cadena_update = "UPDATE puntosobtenidos
							  SET    pob_cantidad_puntos = COALESCE(pob_cantidad_puntos, 0) + $ndc_distincion_academica
							  WHERE  pob_per_codigo = $per_codigo
							  AND    pob_tpu_codigo = 12
							  AND    pob_registro_vigente";
					
            $this->_puntosobtenidos->update($cadenaWhere = "", $rest = "", $interfaz = FALSE, $cadena_update, $mostrarMensaje = FALSE);
		}

		//---Novedad de nuevo salario
		$novedadporpersona = new novedadporpersona ($this->_aplicacion, $this->_usu_login, $this->_grupo, $this->_bd);
        $datos['cds_ten_codigo'] 		= 87;
        $datos['npp_tnp_codigo'] 		= 60;
        $datos["npp_anulada"]    		= $HTTP_POST_VARS["npp_anulada"];
        $datos["npp_pel_codigo"] 		= $HTTP_POST_VARS["npp_pel_codigo"];
        $datos["npp_fecha_novedad"] 	= $HTTP_POST_VARS["npp_fecha_novedad"];
        $datos["npp_descripcion"] 		= $HTTP_POST_VARS["npp_descripcion"];
        $datos["cds_npp_codigo_origen"] = $HTTP_POST_VARS["npp_codigo"];

		//---Calculamos el nuevo salario
		//salario minimo categoria
		$fecha_novedad = strtotime($datos["npp_fecha_novedad"]);
		$SQL = "SELECT smc_salario FROM salariominimocategoriadocentes115
				WHERE smc_cad_codigo = (SELECT cdp_cad_codigo FROM categoriadocentepersona
										WHERE cdp_per_codigo = $per_codigo
										AND   cdp_registro_vigente)
				AND   smc_anno_vigencia = ".date("Y", $fecha_novedad)."
				AND   smc_registro_vigente";
		$salario_minimo_categoria = $this->_consulta->obtenerCadenaResultadoConsulta($SQL);

		//valor del punto
		$SQL = "SELECT vpd_valor_punto
				FROM     valorpuntodocente
				WHERE    vpd_grl_codigo = 1
				ORDER BY vpd_fecha_inicio DESC LIMIT 1";
		$valor_punto = $this->_consulta->obtenerCadenaResultadoConsulta($SQL);

		//puntaje minimo categoria
		$SQL = "SELECT pmc_puntaje FROM puntajeminimocategoriadocentes115
				WHERE pmc_cad_codigo = (SELECT cdp_cad_codigo FROM categoriadocentepersona
										WHERE cdp_per_codigo = $per_codigo
										AND   cdp_registro_vigente)
				AND pmc_registro_vigente
				ORDER BY pmc_fecha_inicio DESC LIMIT 1";
		$puntaje_minimo_categoria = $this->_consulta->obtenerCadenaResultadoConsulta($SQL);

		//puntaje ponderado
		$puntaje_ponderado = $this->calcularPuntajePonderado($per_codigo);

		//dedicacion horaria
		$SQL = "SELECT pca_cantidad_horas_x_semana FROM vinculacionplantacargo, cupoplantacargo, plantacargo
				WHERE    vpc_pel_codigo = $pel_codigo
				AND		 vpc_cpc_codigo = cpc_codigo
				AND 	 cpc_pca_codigo = pca_codigo
                AND 	 cpc_registro_vigente
                AND 	 pca_registro_vigente
                ORDER BY vpc_codigo DESC limit 1";
		$dedicacion_horaria = $this->_consulta->obtenerCadenaResultadoConsulta($SQL);

		$nuevo_salario = $salario_minimo_categoria + $valor_punto * ($puntaje_ponderado - $puntaje_minimo_categoria);

		$nuevo_salario = $nuevo_salario * ($dedicacion_horaria/40);

		//---Consultamos el salario actual
		$SQL = "SELECT 	 cds_valor_nuevo_salario
				FROM   	 cambiodesalario, novedadporpersona
				WHERE 	 cds_npp_codigo = npp_codigo
				AND 	 npp_pel_codigo = $HTTP_POST_VARS[npp_pel_codigo]
				AND 	 cds_registro_vigente
				AND NOT  npp_anulada
				AND 	 novedadaterminadoflujo(npp_codigo)
				AND      npp_codigo <> $npp_codigo
				AND 	 npp_registro_vigente
				ORDER BY npp_fecha_modificacion DESC LIMIT 1";
		$salario_actual = $this->_consulta->obtenerCadenaResultadoConsulta($SQL);

		if ($nuevo_salario >= $salario_actual) {
			$datos["cds_valor_nuevo_salario"] = round($nuevo_salario);
        
			$npp_codigo = $HTTP_POST_VARS['npp_codigo'];

			$novedadporpersona->instanciarClases($datos);
			$novedadporpersona->establecerColumnas($datos);

			if ($HTTP_POST_VARS['descripcion'] == "") {
				$descripcion = 'TERMINADA DESDE EL INGRESO DE LA NOVEDAD';
			} else {
				$descripcion = $HTTP_POST_VARS['descripcion'];
			}

			if ($novedadporpersona->insertar($datos))
			{
				$SQL = "UPDATE  estadonovedadnodo
						SET     enn_tea_nombre = 'TERMINADA',
								enn_descripcion = '$descripcion'
						WHERE   enn_npp_codigo = $datos[npp_codigo]
								AND enn_tea_nombre = 'EN ESPERA'
								AND enn_registro_vigente";

				if ($this->_puntosobtenidos->update($cadenaWhere = "", $rest = "", $interfaz = FALSE, $SQL, $mostrarMensaje = FALSE))
				{
					$HTTP_POST_VARS['descripcion']='INGRESADA DESDE LA NOVEDAD DE NIVELACION DOCENTE 115';
					return TRUE;
				}
				else
				{
					return FALSE;
				}
			}
		}
        else
        {
            return FALSE;
        }
        //---Fin de ingreso de la novedad de salario
		
    }



	/**
	 * Calcula el puntaje ponderado de un docente
     *
     * @access      public
     * @author      Cristhian David Marín <cristhian.marin@correounivalle.edu.co>
     * @copyright   OITEL, Universidad del Valle 2014
     * @return      float
	 */
	function calcularPuntajePonderado (&$per_codigo)
	{
		$sql = "SELECT pob_tpu_codigo, pob_cantidad_puntos FROM puntosobtenidos
				WHERE pob_per_codigo = $per_codigo
				AND   pob_registro_vigente";
		$this->_consulta->ejecutarConsulta($sql);

		$puntaje_ponderado = 0;
		while (list($tipo_punto, $cantidad_puntos) = $this->_consulta->sigRegistroArreglo()) {

			$sql = "SELECT tpu_porcentaje FROM tipopunto
					WHERE  tpu_codigo = $tipo_punto
					AND	   tpu_registro_vigente";
			$cantidad_puntos = $cantidad_puntos * $this->_consulta_aux->obtenerCadenaResultadoConsulta($sql);

			$puntaje_ponderado = $puntaje_ponderado + $cantidad_puntos;
		}

		return $puntaje_ponderado;
	}



    /**
     * Este método valida dos aspectos referentes al nuevo salario del empleado:
     * - Verifica que exista un salario para el grado seleccionado
     * - Si dicho salario existe verifica que este salario sea mayor o igual que el
     *   salario anterior.
     *
     * @access      public
     * @author      Cristhian David Marín <cristhian.marin@correounivalle.edu.co>
     * @copyright   OITEL, Universidad del Valle 2.014
     * @return      boolean
     */

    function validarSalarioDelCargo ($HTTP_POST_VARS)
    {
        $gra_codigo = $HTTP_POST_VARS["niv_gra_codigo"];
        $pel_codigo = $HTTP_POST_VARS["npp_pel_codigo"];

        $SQL = "SELECT  pel_grl_codigo FROM personaliquidacion WHERE pel_codigo = $pel_codigo";
        $grl_codigo = $this->_consulta->obtenerCadenaResultadoConsulta($SQL);

        switch($grl_codigo)
        {
            case PUBLICO:
                $SQL = "SELECT  pca_car_codigo
                        FROM    vinculacionplantacargo, cupoplantacargo, plantacargo
                        WHERE   vpc_pel_codigo = $pel_codigo AND
                                cpc_codigo = vpc_cpc_codigo  AND
                                cpc_pca_codigo = pca_codigo  AND
                                cpc_registro_vigente AND
                                pca_registro_vigente AND
                                vpc_registro_vigente
                        ORDER BY vpc_fecha_inicio
                        LIMIT 1";

                $car_codigo = $this->_consulta->obtenerCadenaResultadoConsulta($SQL);
                $SQL = "SELECT  sbp_salario
                        FROM    salariobasicoempleadospublicos
                        WHERE   sbp_car_codigo = '$car_codigo' AND
                                sbp_gra_codigo = '$gra_codigo' AND
                                sbp_registro_vigente";
            break;
            case SUPERNUMERARIO:
                $SQL = "
                    SELECT  vsn_pel_codigo_a_reemplazar
                    FROM    novedadporpersona,vinculacionsupernumerario
                    WHERE   vsn_npp_codigo=npp_codigo
                      AND   npp_tnp_codigo=27
                      AND   npp_pel_codigo=$pel_codigo
                      AND   NOT npp_anulada
                      AND   novedadaterminadoflujo(npp_codigo)
                      AND   npp_registro_vigente
                      AND   vsn_registro_vigente
                ";
                $pel_codigo_a_reemplazar = $this->_consulta->obtenerCadenaResultadoConsulta($SQL);
                $SQL = "SELECT  pca_car_codigo
                        FROM    vinculacionplantacargo, cupoplantacargo, plantacargo
                        WHERE   vpc_pel_codigo = $pel_codigo_a_reemplazar AND
                                cpc_codigo = vpc_cpc_codigo  AND
                                cpc_pca_codigo = pca_codigo  AND
                                cpc_registro_vigente AND
                                pca_registro_vigente AND
                                vpc_registro_vigente
                        ORDER BY vpc_fecha_inicio
                        LIMIT 1";

                $car_codigo = $this->_consulta->obtenerCadenaResultadoConsulta($SQL);

                $SQL = "SELECT  sbp_salario
                        FROM    salariobasicoempleadospublicos
                        WHERE   sbp_car_codigo = '$car_codigo' AND
                                sbp_gra_codigo = '$gra_codigo' AND
                                sbp_registro_vigente";
            break;
            case OFICIAL_CON_PREST_DE_LEY:
            case OFICIAL_CONVENCION_COLEC:
                $SQL = "SELECT  pca_car_codigo
                        FROM    vinculacionplantacargo, cupoplantacargo, plantacargo
                        WHERE   vpc_pel_codigo = $pel_codigo AND
                                cpc_codigo = vpc_cpc_codigo  AND
                                cpc_pca_codigo = pca_codigo  AND
                                cpc_registro_vigente AND
                                pca_registro_vigente AND
                                vpc_registro_vigente
                        ORDER BY vpc_fecha_inicio
                        LIMIT 1";

                $car_codigo = $this->_consulta->obtenerCadenaResultadoConsulta($SQL);

                $SQL = "SELECT  sbo_salario
                        FROM    salariobasicotrabajadoresoficiales
                        WHERE   sbo_car_codigo = '$car_codigo' AND
                                sbo_gra_codigo = '$gra_codigo' AND
                                sbo_registro_vigente";
            break;
            case CONTRATISTA_A_TERMINO_FIJO:
                $SQL = "
                    SELECT  vtf_pel_codigo_a_reemplazar
                    FROM    novedadporpersona,vinculacioncontratistaterminofijo
                    WHERE   vtf_npp_codigo=npp_codigo
                      AND   npp_tnp_codigo=27
                      AND   npp_pel_codigo=$pel_codigo
                      AND   NOT npp_anulada
                      AND   novedadaterminadoflujo(npp_codigo)
                      AND   npp_registro_vigente
                      AND   vtf_registro_vigente
                ";
                $pel_codigo_a_reemplazar = $this->_consulta->obtenerCadenaResultadoConsulta($SQL);
                $SQL = "SELECT  pca_car_codigo
                        FROM    vinculacionplantacargo, cupoplantacargo, plantacargo
                        WHERE   vpc_pel_codigo = $pel_codigo_a_reemplazar AND
                                cpc_codigo = vpc_cpc_codigo  AND
                                cpc_pca_codigo = pca_codigo  AND
                                cpc_registro_vigente AND
                                pca_registro_vigente AND
                                vpc_registro_vigente
                        ORDER BY vpc_fecha_inicio
                        LIMIT 1";

                $car_codigo = $this->_consulta->obtenerCadenaResultadoConsulta($SQL);

                $SQL = "SELECT  sbo_salario
                        FROM    salariobasicotrabajadoresoficiales
                        WHERE   sbo_car_codigo = '$car_codigo' AND
                                sbo_gra_codigo = '$gra_codigo' AND
                                sbo_registro_vigente";
            break;
            default:
                return FALSE;
            break;
        }

        $this->_consulta->ejecutarConsulta($SQL);
        if ($this->_consulta->numFilas() <= 0)
        {
            $error = new error("ERROR","ERROR919");
            $mesg = $error->retornarMensajeError();
            $error->imprimirMesajeEnJS($mesg);
            print "<center>$mesg</center>";

            return FALSE;
        }
        else
        {
            list($new_salario) = $this->_consulta->sigRegistroArreglo();
        }


        //Si llega aquí es por que existe un Salario, ahora se valida el salario antes Vs el actual.

        switch($grl_codigo)
        {
            case PUBLICO:
                $SQL = "
                    SELECT  cds_valor_nuevo_salario
                    FROM    cambiodesalario, novedadporpersona
                    WHERE   cds_npp_codigo = npp_codigo  AND
                            npp_pel_codigo = $pel_codigo AND
                            NOT npp_anulada      AND
                            npp_registro_vigente AND
                            cds_registro_vigente AND
                            novedadaterminadoflujo(npp_codigo)

                    ORDER BY npp_fecha_novedad desc
                    LIMIT 1";
                $this->_consulta->ejecutarConsulta($SQL);
                if($this->_consulta->numFilas() == 0)
                {
                    $SQL = "SELECT  vpb_gra_codigo
                            FROM    vinculacionpublico, vinculacionlaboral, novedadporpersona
                            WHERE   npp_pel_codigo = '$pel_codigo' AND
                                    vpb_vla_codigo = vla_codigo AND
                                    vla_npp_codigo = npp_codigo AND
                                    vpb_registro_vigente AND
                                    npp_registro_vigente AND
                                    NOT npp_anulada";

                    $old_gra_codigo = $this->_consulta->obtenerCadenaResultadoConsulta($SQL);
                    $SQL = "SELECT  sbp_salario
                            FROM    salariobasicoempleadospublicos
                            WHERE   sbp_car_codigo = $car_codigo AND
                                    sbp_gra_codigo = $old_gra_codigo AND
                                    sbp_registro_vigente";

                    $old_salario = $this->_consulta->obtenerCadenaResultadoConsulta($SQL);
                }
                else
                {
                    list($old_salario) = $this->_consulta->sigRegistroArreglo($SQL);
                }
            break;
            case SUPERNUMERARIO:
            case CONTRATISTA_A_TERMINO_FIJO:
            case OFICIAL_CON_PREST_DE_LEY:
            case OFICIAL_CONVENCION_COLEC:
                $SQL = "SELECT  cds_valor_nuevo_salario
                        FROM    cambiodesalario, novedadporpersona
                        WHERE   cds_npp_codigo = npp_codigo  AND
                                npp_pel_codigo = $pel_codigo AND
                                npp_registro_vigente AND
                                cds_registro_vigente AND
                                NOT npp_anulada      AND
                                novedadaterminadoflujo(npp_codigo)

                        ORDER BY npp_fecha_novedad DESC
                        LIMIT 1";
                $old_salario = $this->_consulta->obtenerCadenaResultadoConsulta($SQL);
            break;
            default:
                return FALSE;
            break;
        }

        if ($new_salario <= $old_salario)
        {
            $error = new error("ERROR","ERROR925");
            $mesg = $error->retornarMensajeError();
            $error->imprimirMesajeEnJS($mesg);
            print "<center>$mesg</center>";

            return FALSE;
        }
        return TRUE;
    }


    /**
     * Permite hacer la validación los datos básicos.
     *
     * @access      public
     * @author      Cristhian David Marín <cristhian.marin@correounivalle.edu.co>
     * @copyright   OITEL, Universidad del Valle 2014
     * @return      boolean
     */

    function validarAtributosAvanzados (&$HTTP_POST_VARS)
    {
        $respuesta = TRUE;

        if ($HTTP_POST_VARS['npp_pel_codigo'] == "")
        {
            $error = new error("ERROR","ERROR871");
            $respuesta = FALSE;
        }
        
        if ($this->todosLosPuntajesCero($HTTP_POST_VARS))
        {
			$error = new error("ERROR", "ERROR1101");
			$respuesta = FALSE;
		}

        if ($this->existeFechaNovedadIgual($HTTP_POST_VARS))
        {
			$error = new error("ERROR", "ERROR606");
			$respuesta = FALSE;
		}

        if (!$respuesta)
        {
            $mesg = $error->retornarMensajeError();
            eval( "\$mesg = \"$mesg\";" );
            print "<br><center>$mesg</center><br>";
        }

        return $respuesta;

    }//END validarAtributosAvanzados
    
    
    
    /**
     * Valida si todos los puntos de la novedad son 0 (ningun cambio)
     *
     * @access      public
     * @author      Cristhian David Marín <cristhian.marin@correounivalle.edu.co>
     * @copyright   OITEL, Universidad del Valle 2014
     * @return      boolean
     */
    
    function todosLosPuntajesCero($HTTP_POST_VARS)
    {
		if ($HTTP_POST_VARS['ndc_titulos_obtenidos'] == 0 && $HTTP_POST_VARS['ndc_labor_docente'] == 0 &&
			$HTTP_POST_VARS['ndc_experiencia_profesional'] == 0 && $HTTP_POST_VARS['ndc_experiencia_academica'] == 0 &&
			$HTTP_POST_VARS['ndc_produccion_intelectual'] == 0 && $HTTP_POST_VARS['ndc_idiomas'] == 0 &&
			$HTTP_POST_VARS['ndc_distincion_academica'] == 0)
			return TRUE;
		else
			return FALSE;
	}
    
    
    
    /**
     * Valida si ya existe una nivelacion con una fecha igual para dicha persona
     *
     * @access      public
     * @author      Cristhian David Marín <cristhian.marin@correounivalle.edu.co>
     * @copyright   OITEL, Universidad del Valle 2014
     * @return      boolean
     */
    
	function existeFechaNovedadIgual($HTTP_POST_VARS)
	{
		$fecha_novedad = $HTTP_POST_VARS["npp_fecha_novedad"];
		$pel_codigo = $HTTP_POST_VARS["npp_pel_codigo"];

		$SQL = "SELECT pel_per_codigo 
		        FROM   personaliquidacion 
		        WHERE  pel_codigo = $pel_codigo
		        AND    pel_registro_vigente";
		        
        $per_codigo = $this->_consulta->obtenerCadenaResultadoConsulta($SQL); //Obtengo la persona
        
        $SQL = "SELECT  npp_fecha_novedad
				FROM	novedadporpersona, nivelaciondocentecientoquince, personaliquidacion
				WHERE	npp_codigo = ndc_npp_codigo
				AND		novedadaterminadoflujo(npp_codigo)
				AND		npp_pel_codigo = pel_codigo
				AND		pel_per_codigo = $per_codigo
				AND		npp_fecha_novedad = '$fecha_novedad'
				AND NOT	npp_anulada
				AND		npp_registro_vigente
				AND		ndc_registro_vigente
				AND		pel_registro_vigente";
		$this->_consulta->ejecutarConsulta($SQL);
		
		if ($this->_consulta->numFilas() > 0)
			return TRUE;
		else
			return FALSE;
	}
    
    
    /**
    * Valida si existe un registro de novedad de nivelacion con ese mismos grado
    * que este en ESPERA o TERMINADA, para una persona en particular y para un 
    * periodo en el que se desea ingresar la novedad.              
    *
    * @author      Cristhian David Marín <cristhian.marin@correounivalle.edu.co>
    * @access      public
    * @param       $HTTP_POST_VARS         POST VARS
    * @copyright   OITEL, Universidad del Valle 2.014
    * @return      boolean
    */
    function validarExistenteRegistro($HTTP_POST_VARS){
        //$mes = substr($HTTP_POST_VARS[npp_fecha_novedad], 5, 2);
        //$anho = substr($HTTP_POST_VARS[npp_fecha_novedad], 0, 4);
        
        $SQL = "
            SELECT  *
            FROM    novedadporpersona,
                  	estadonovedadnodo,
                  	nivelaciondocesietenueve
            WHERE	npp_tnp_codigo = $HTTP_POST_VARS[npp_tnp_codigo]
                	AND enn_npp_codigo = npp_codigo
                	AND (enn_tea_nombre = 'TERMINADA' OR enn_tea_nombre = 'EN ESPERA')
                	AND niv1279_npp_codigo = npp_codigo
                	AND niv1279_gra_codigo = $HTTP_POST_VARS[niv_gra_codigo]
                	AND npp_pel_codigo = $HTTP_POST_VARS[npp_pel_codigo]
                	--AND npp_fecha_novedad LIKE '$anho-$mes%'
                	AND NOT npp_anulada
                	AND npp_registro_vigente
                	AND niv1279_registro_vigente
                	AND enn_registro_vigente;
        ";
        $this->_consulta->ejecutarConsulta($SQL);
        if($this->_consulta->numFilas()>0){
            return TRUE;
        }
        
        return FALSE;
    }//END validarExistenteRegistro
    
    
    /**
    * Anula la novedad de cambio de Salario
    *
    * @author      Cristhian David Marín <cristhian.marin@correounivalle.edu.co>
    * @access      public
    * @param       $HTTP_POST_VARS         POST VARS
    * @copyright   OITEL, Universidad del Valle 2.014
    * @return      boolean
    */
    function realizarProcedimientosAnterioresALaAnulacion (&$HTTP_POST_VARS, $verificar_novedadEstaEnEsperaPorUsuario = FALSE){
        $respuesta = FALSE;
        
        $SQL = "SELECT novedadaterminadoflujo($HTTP_POST_VARS[npp_codigo])";

        $esta_terminada = $this->_consulta->obtenerCadenaResultadoConsulta($SQL); //Busco el estado de esta novedad

        if ($esta_terminada == "t") { //Garantizo que ya este terminada para proceder a cancelar todos los cambios hechos

			$this->_puntosobtenidos = new TABLA("puntosobtenidos",$this->_usu_login, $this->_grupo, $this->_bd);
			$pel_codigo = $HTTP_POST_VARS["npp_pel_codigo"];

			$SQL = "SELECT pel_per_codigo 
					FROM   personaliquidacion 
					WHERE  pel_codigo = $pel_codigo
					AND    pel_registro_vigente";
		        
			$per_codigo = $this->_consulta->obtenerCadenaResultadoConsulta($SQL); //Obtengo la persona

			//Ahora dependiendo de que campo se añadieron puntos, revierto la cantidad de puntos del afectado
			if ($HTTP_POST_VARS['ndc_titulos_obtenidos'] != 0) {
				$ndc_titulos_obtenidos = $HTTP_POST_VARS['ndc_titulos_obtenidos'];
				$cadena_update = "UPDATE puntosobtenidos
								  SET    pob_cantidad_puntos = COALESCE(pob_cantidad_puntos, 0) - $ndc_titulos_obtenidos
								  WHERE  pob_per_codigo = $per_codigo
								  AND    pob_tpu_codigo = 6
								  AND    pob_registro_vigente";
					
				$this->_puntosobtenidos->update($cadenaWhere = "", $rest = "", $interfaz = FALSE, $cadena_update, $mostrarMensaje = FALSE);
			}

			if ($HTTP_POST_VARS['ndc_labor_docente'] != 0) {
				$ndc_labor_docente = $HTTP_POST_VARS['ndc_labor_docente'];
				$cadena_update = "UPDATE puntosobtenidos
								  SET    pob_cantidad_puntos = COALESCE(pob_cantidad_puntos, 0) - $ndc_labor_docente
								  WHERE  pob_per_codigo = $per_codigo
								  AND    pob_tpu_codigo = 7
								  AND    pob_registro_vigente";
					
				$this->_puntosobtenidos->update($cadenaWhere = "", $rest = "", $interfaz = FALSE, $cadena_update, $mostrarMensaje = FALSE);
			}

			if ($HTTP_POST_VARS['ndc_experiencia_profesional'] != 0) {
				$ndc_experiencia_profesional = $HTTP_POST_VARS['ndc_experiencia_profesional'];
				$cadena_update = "UPDATE puntosobtenidos
								  SET    pob_cantidad_puntos = COALESCE(pob_cantidad_puntos, 0) - $ndc_experiencia_profesional
								  WHERE  pob_per_codigo = $per_codigo
								  AND    pob_tpu_codigo = 8
								  AND    pob_registro_vigente";
					
				$this->_puntosobtenidos->update($cadenaWhere = "", $rest = "", $interfaz = FALSE, $cadena_update, $mostrarMensaje = FALSE);
			}

			if ($HTTP_POST_VARS['ndc_experiencia_academica'] != 0) {
				$ndc_experiencia_academica = $HTTP_POST_VARS['ndc_experiencia_academica'];
				$cadena_update = "UPDATE puntosobtenidos
								  SET    pob_cantidad_puntos = COALESCE(pob_cantidad_puntos, 0) - $ndc_experiencia_academica
								  WHERE  pob_per_codigo = $per_codigo
								  AND    pob_tpu_codigo = 9
								  AND    pob_registro_vigente";
					
				$this->_puntosobtenidos->update($cadenaWhere = "", $rest = "", $interfaz = FALSE, $cadena_update, $mostrarMensaje = FALSE);
			}

			if ($HTTP_POST_VARS['ndc_produccion_intelectual'] != 0) {
				$ndc_produccion_intelectual = $HTTP_POST_VARS['ndc_produccion_intelectual'];
				$cadena_update = "UPDATE puntosobtenidos
								  SET    pob_cantidad_puntos = COALESCE(pob_cantidad_puntos, 0) - $ndc_produccion_intelectual
								  WHERE  pob_per_codigo = $per_codigo
								  AND    pob_tpu_codigo = 10
								  AND    pob_registro_vigente";
					
				$this->_puntosobtenidos->update($cadenaWhere = "", $rest = "", $interfaz = FALSE, $cadena_update, $mostrarMensaje = FALSE);
			}

			if ($HTTP_POST_VARS['ndc_idiomas'] != 0) {
				$ndc_idiomas = $HTTP_POST_VARS['ndc_idiomas'];
				$cadena_update = "UPDATE puntosobtenidos
								  SET    pob_cantidad_puntos = COALESCE(pob_cantidad_puntos, 0) - $ndc_idiomas
								  WHERE  pob_per_codigo = $per_codigo
								  AND    pob_tpu_codigo = 11
								  AND    pob_registro_vigente";
					
				$this->_puntosobtenidos->update($cadenaWhere = "", $rest = "", $interfaz = FALSE, $cadena_update, $mostrarMensaje = FALSE);
			}

			if ($HTTP_POST_VARS['ndc_distincion_academica'] != 0) {
				$ndc_distincion_academica = $HTTP_POST_VARS['ndc_distincion_academica'];
				$cadena_update = "UPDATE puntosobtenidos
								  SET    pob_cantidad_puntos = COALESCE(pob_cantidad_puntos, 0) - $ndc_distincion_academica
								  WHERE  pob_per_codigo = $per_codigo
								  AND    pob_tpu_codigo = 12
								  AND    pob_registro_vigente";
					
				$this->_puntosobtenidos->update($cadenaWhere = "", $rest = "", $interfaz = FALSE, $cadena_update, $mostrarMensaje = FALSE);
			}

		}
		
		return TRUE;
        
    }//END realizarProcedimientosAnterioresALaAnulacion



	 /**
    * Anula la novedad de cambio de Salario
    *
    * @author      Jhonatan Muñoz Morales <jhonatan.munoz@correounivalle.edu.co>
    * @access      public
    * @param       $HTTP_POST_VARS         POST VARS
    * @copyright   OITEL, Universidad del Valle 2011
    * @since       2011-11-01
    * @return      boolean
    */
    function realizarOperacionesDeAnulacionNovedad (&$HTTP_POST_VARS, $verificar_novedadEstaEnEsperaPorUsuario=FALSE){
        $respuesta = FALSE;
        
        $SQL = "SELECT  cds_npp_codigo
				FROM    cambiodesalario
				WHERE   cds_npp_codigo_origen = $HTTP_POST_VARS[npp_codigo]
				AND 	cds_registro_vigente";

        $npp_codigo = $this->_consulta->obtenerCadenaResultadoConsulta($SQL);
        if ($npp_codigo != "") {
            $DATOS['cds_npp_codigo'] = $npp_codigo;
            $DATOS['npp_codigo'] = $npp_codigo;
            $DATOS['npp_anulada']             = "f";
            $DATOS['npp_descripcion_anulacion'] = "ANULADA DESDE LA NOVEDAD DE NIVELACION";
            
            $this->_clasenovedadporpersona->instanciarClases($DATOS);
            $this->_clasenovedadporpersona->establecerColumnas($DATOS);
            
            if ($this->_clasenovedadporpersona->sePuedeAprobar($verificar_novedadEstaEnEsperaPorUsuario)) {
                if ($this->_clasenovedadporpersona->realizarProcedimientosAnterioresALaAnulacion($DATOS)) {
                    if ($this->_clasenovedadporpersona->_estadonovedadnodo->anularNovedadNodo($DATOS)) {
                        if ($this->_clasenovedadporpersona->anular($DATOS))
                            $respuesta = $this->_clasenovedadporpersona->realizarOperacionesDeAnulacionNovedad($DATOS);
                    }
                }
            } else {
				$error = new error("ERROR","ERROR717");
				$mesg = $error->retornarMensajeError();
				eval( "\$mesg = \"$mesg\";" );
				$error->imprimirMesajeEnJS ($mesg);
				print "<center>$mesg</center>";
				return false;
            }
        }
        
        return parent::realizarOperacionesDeAnulacionNovedad($HTTP_POST_VARS);
        
    }//END realizarOperacionesDeAnulacionNovedad
} // End: class nivelacion extends TablaMaestro

?>
