/* also: Advanced Logic Synthesis and Optimization tool
 * Copyright (C) 2019- Ningbo University, Ningbo, China */

/**
 * @file xag_dec.hpp
 *
 * @brief Deompose a truth table into an XAG signal by combined
 * decompostion methods (DSD, Shannon, and NPN )
 *
 * @author Zhufei Chu
 * @since  0.1
 */

#ifndef XAG_DEC_HPP
#define XAG_DEC_HPP

namespace also
{
  
  template<class Ntk>
  class xag_dec_impl
  {
    public:
      xag_dec_impl( Ntk& ntk, kitty::dynamic_truth_table const& func, std::vector<signal<Ntk>> const& children, dsd_decomposition_params const& ps )
        : _ntk( ntk ),
          _func( func ),
          pis( children ),
          _ps( ps )
      {
        for ( auto i = 0u; i < func.num_vars(); ++i )
        {
          if ( kitty::has_var( func, i ) )
          {
            support.push_back( i );
          }
        }
      }

      signal<Ntk> decompose( kitty::dynamic_truth_table& remainder )
      {
        /* step 1: check constants */
        if ( kitty::is_const0( remainder ) )
        {
          return _ntk.get_constant( false );
        }
        if ( kitty::is_const0( ~remainder ) )
        {
          return _ntk.get_constant( true );
        }

        /* step 2: check primary inputs*/
        if ( support.size() == 1u )
        {
          auto var = remainder.construct();
          kitty::create_nth_var( var, support.front() );
          if ( remainder == var )
          {
            return pis[support.front()];
          }
          else
          {
            if ( remainder != ~var )
            {
              fmt::print( "remainder = {}, vars = {}\n", kitty::to_binary( remainder ), remainder.num_vars() );
              assert( false );
            }
            assert( remainder == ~var );
            return _ntk.create_not( pis[support.front()] );
          }
        }

        /* step 3: check top disjoint decomposition */
        for ( auto var : support )
        {
          if ( auto res = kitty::is_top_decomposable( remainder, var, &remainder, _ps.with_xor );
              res != kitty::top_decomposition::none )
          {
            std::cout << "Top decomposition on var " << +var << " is successful\n";
            /* remove var from support, pis do not change */
            support.erase( std::remove( support.begin(), support.end(), var ), support.end() );
            const auto right = decompose( remainder );

            switch ( res )
            {
              default:
                assert( false );
              case kitty::top_decomposition::and_:
                return _ntk.create_and( pis[var], right );
              case kitty::top_decomposition::or_:
                return _ntk.create_or( pis[var], right );
              case kitty::top_decomposition::lt_:
                return _ntk.create_lt( pis[var], right );
              case kitty::top_decomposition::le_:
                return _ntk.create_le( pis[var], right );
              case kitty::top_decomposition::xor_:
                return _ntk.create_xor( pis[var], right );
            }
          }
        }

        /* step 4: check bottom disjoint decomposition */
        for ( auto j = 1u; j < support.size(); ++j )
        {
          for ( auto i = 0u; i < j; ++i )
          {
            std::cout << "Bottom decomposition on var " << +support[i] << " and " << +support[j] << " is successful\n";
            if ( auto res = kitty::is_bottom_decomposable( remainder, support[i], support[j], &remainder, _ps.with_xor );
                res != kitty::bottom_decomposition::none )
            {
              /* update pis based on decomposition type */
              switch ( res )
              {
                default:
                  assert( false );
                case kitty::bottom_decomposition::and_:
                  pis[support[i]] = _ntk.create_and( pis[support[i]], pis[support[j]] );
                  break;
                case kitty::bottom_decomposition::or_:
                  pis[support[i]] = _ntk.create_or( pis[support[i]], pis[support[j]] );
                  break;
                case kitty::bottom_decomposition::lt_:
                  pis[support[i]] = _ntk.create_lt( pis[support[i]], pis[support[j]] );
                  break;
                case kitty::bottom_decomposition::le_:
                  pis[support[i]] = _ntk.create_le( pis[support[i]], pis[support[j]] );
                  break;
                case kitty::bottom_decomposition::xor_:
                  pis[support[i]] = _ntk.create_xor( pis[support[i]], pis[support[j]] );
                  break;
              }

              /* remove var from support */
              support.erase( support.begin() + j );

              return decompose( remainder ); 
            }
          }
        }

        /* step 5: for a prime function, if the number of vars is
         * larger than 4u, apply shannon decomposition, else invoke NPN */

        if( support.size() > 4u )
        {
          /* TODO: shannon decomposition */
          
        }
        else
        {
          /* TODO: npn */
        }

      }

      signal<Ntk> run()
      {
        return decompose( _func );
      }

    private:
    Ntk& _ntk;
    kitty::dynamic_truth_table _func;
    std::vector<uint8_t> support;
    std::vector<signal<Ntk>> pis;
    dsd_decomposition_params const& _ps;

  };

  template<class Ntk>
    signal<Ntk> xag_dec( Ntk& ntk, kitty::dynamic_truth_table const& func, std::vector<signal<Ntk>> const& children, dsd_decomposition_params const& ps = {} )
    {
      xag_dec_impl<Ntk> impl( ntk, func, children, ps );
      return impl.run();
    }


} //end of namespace

#endif
